#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>
#include <list>
#include <map>         // For std::map (used in bids / asks)
#include <vector>      // For std::vector (used in LevelInfos, Trades)
#include <cstdint>     // For std::int32_t, std::uint32_t, std::uint64_t
#include <stdexcept>   // For std::logic_error
#include <numeric>


// scoped enumeration (C++ 11), need to use the OrderType::... to access elements
enum class OrderType
{
	// order remains active indefinitely until its completely filled, or explicitly cancelled by the user
	GoodTillCancel,
	
	// Filled upon arrival, whatever quantity cannot be filled immediately is killed on the spot
	FillAndKill
};

enum class Side
{
	Buy,
	Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo
{
	Price price;
	Quantity quantity;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{
	public:
		OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks): bids{bids}, asks{asks} {}

		const LevelInfos& GetBids() const {return bids;} 
		const LevelInfos& GetAsks() const {return asks;}
	
	private:
		LevelInfos bids;
		LevelInfos asks;
};

class Order
{
	public:
		Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
			: orderType{orderType}, 
			orderId{orderId}, 
			side{side}, 
			price{price}, 
			initialQuantity{quantity},
			remainingQuantity{quantity}
		{ }


		OrderId GetOrderId() const {return orderId; }
		Side GetSide() const { return side; }
		OrderType GetOrderType() const {return orderType; }
		Price GetPrice() const {return price; }
		Quantity GetInitialQuantity() const {return initialQuantity; }
		Quantity GetRemainingQuantity() const {return remainingQuantity; }
		Quantity GetFilledQuantity() const {return initialQuantity - remainingQuantity; }

		bool IsFilled() const {return remainingQuantity == 0; }
		void Fill(Quantity quantity){
			if(quantity > GetRemainingQuantity() == 0)
				remainingQuantity -= quantity;
			else throw std::logic_error("Can't overfill an order");
		}

		void CancelOrder(){}


		       

	private:
		OrderType orderType;
		OrderId orderId;
		Side side;
		Price price;
		// really only matters for the GoodTillCancel orders
		Quantity initialQuantity;
		Quantity remainingQuantity;

};




// i'm going to alias individual order pointers and lists of orderpointers in order to not have to rewrite the long identifier 

// allows me to not have to worry about dangling pointers
// 	going to try and remove the need for the shared ptrs
// 	for atomic instruction, malloc, I want to prefer flat, contiguous memory
using OrderPointer = std::shared_ptr<Order>;

// OrderPointers is a doubly linked list, takes more memory, but faster operations, like cancellations
using OrderPointers = std::list<OrderPointer>;


// class for the API
class OrderModify
{
	public:
		OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
			: orderId{orderId}, 
			price{price},
			quantity{quantity}{}	
	
		OrderId GetOrderId() const {return orderId; }
		Price GetPrice() const {return price;}
		Side GetSide() const {return side;}
		Quantity GetQuantity() const{return quantity;}

		OrderPointer ToOrderPointer(OrderType type) const{
			return std::make_shared<Order> (type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
		}
	
	private:
		OrderId orderId;
		Side side;
		Price price;
		Quantity quantity;
};

// what do we do when an order is matched?
struct TradeInfo{
	OrderId orderId;
	Price price;
	Quantity quantity;
};

class Trade{
	public:
		Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
			: bidTrade{bidTrade},
			askTrade{askTrade} 
		{}

		const TradeInfo& GetBidTrade() const {return bidTrade;}
		const TradeInfo& GetAskTrade() const {return askTrade;}
	
	private:
		TradeInfo bidTrade;
		TradeInfo askTrade;
};

// now I actually need to make the orderbook
// To note:
// how do we store our maps, maps or unorderped maps
// we're going to use map to store our bids, because we can sort the bids
// order and location we will represent with an orderentry obj

using Trades = std::vector<Trade>;


class Orderbook{

	private:
		struct OrderEntry
		{
			OrderPointer order{nullptr};
			// iterators in std::list allow me to do O(1) cancellations
			OrderPointers::iterator location;
		};
        	
		// orders are sorted by price, then time
		// 	prioritize GREATEST bids
		// 	prioritize LOWEST asks
		//
		// map that maps each price to a list of orders
		
		std::map<Price, OrderPointers, std::greater<Price>> bids;
		std::map<Price, OrderPointers, std::less<Price>> asks;
		std::unordered_map<OrderId, OrderEntry> orders;

		// order came in, can I match the request
		bool CanMatch(Side side, Price price) const{
			
			// order that came in is a Buy
			if(side == Side::Buy){
				// we have no asks
				if(asks.empty()) return false;
			
				// we choose asks with the lowest price
				// taking just the value of the bestAsk
				const auto& [bestAsk, _] = *asks.begin();
				return price >= bestAsk;
			}
			// order that came in is a Sell
			else{
				if(bids.empty()) return false;

				const auto& [bestBid, _] = *bids.begin();
				return price <= bestBid;
			}
		}

		
		Trades MatchOrders(){

			Trades trades;
			// reserve in memory?
			trades.reserve(orders.size());

			while(true){
				if(bids.empty() || asks.empty()) break;

				auto& [bidPrice, bidsAtbidPrice] = *this->bids.begin();
				auto& [askPrice, asksAtaskPrice] = *this->asks.begin();

				if(bidPrice < askPrice) break;

				while(bidsAtbidPrice.size() && asksAtaskPrice.size()){

					auto& bid = bidsAtbidPrice.front();
					auto& ask = asksAtaskPrice.front();

					Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());
					bid->Fill(quantity);
					ask->Fill(quantity);


					// make the trade
					trades.push_back(Trade{
							TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity }, 
							TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}});


					if(bid->IsFilled()){
						// take the bid off the OrderPointers list and destroy the order pointer
						bidsAtbidPrice.pop_front();
						orders.erase(bid->GetOrderId());
					}
					
					if(ask->IsFilled()){
						// take the ask off the OrderPointers list and destroy the order pointer
						asksAtaskPrice.pop_front();
						orders.erase(ask->GetOrderId());
					}

					// if I have no more bids/asks at the price level, then eliminate the price level
					if(bidsAtbidPrice.empty()) bids.erase(bidPrice);
					
					if(asksAtaskPrice.empty()) asks.erase(askPrice);

				}

			}


			// we need to handle FillandKill orders
			// To note:
			// 	We throw away FAK orders that don't cross (match the other order type)
			if(!bids.empty()){
				auto& [_, bids] = *this->bids.begin();
				auto& order = bids.front();
				if(order->GetOrderType() == OrderType::FillAndKill)
					CancelOrder(order->GetOrderId());
			}
					
			if(!asks.empty()){
				auto& [_, asks] = *this->asks.begin();
				auto& order = asks.front();
				if(order->GetOrderType() == OrderType::FillAndKill)
					CancelOrder(order->GetOrderId());
			}

			return trades;
		}


	public:

		Trades AddOrder(OrderPointer newOrder){

			if(orders.contains(newOrder->GetOrderId()))
				return {};

			// handle FillAndKill orders that don't match any prices
			if(newOrder->GetOrderType() == OrderType::FillAndKill && !CanMatch(newOrder->GetSide(), newOrder->GetPrice()))
				return {};
			
			// we're storing this iterator that points to the orders list for O(1) order cancellation
			OrderPointers::iterator iterator;
			
			
			// we want to get the pointer to the list at the order's price level
			// I want to insert 


			if (newOrder->GetSide() == Side::Buy){
				auto& orders = this->bids[newOrder->GetPrice()];
				orders.push_back(newOrder);
				iterator = std::next(orders.begin(), orders.size()-1);
			}
			else{
				auto& orders = this->asks[newOrder->GetPrice()];
				orders.push_back(newOrder);
				iterator = std::next(orders.begin(), orders.size()-1);
			}
			
			orders.insert({newOrder->GetOrderId(), OrderEntry{newOrder, iterator}});
			return MatchOrders();
		}



		void CancelOrder(OrderId orderId){

			if(!orders.contains(orderId)) return;

			const auto& [order, orderIterator] = orders.at(orderId);
			
			if(order->GetSide() == Side::Sell){
				auto price = order->GetPrice();
				// list of asks at a certain price, i want to take the orderId order off it
				auto& order = asks.at(price);
				order.erase(orderIterator);
				if(order.empty())
					asks.erase(price);
			}
			else{
				auto price = order->GetPrice();
				// list of bids at a certain price, i want to take the orderId order off it
				auto& order = bids.at(price);
				order.erase(orderIterator);
				if(orders.empty())
					bids.erase(price);
			}

			orders.erase(orderId);
		}

		Trades MatchOrder(OrderModify order)
		{
			if(!orders.contains(order.GetOrderId()))
				return {};

			const auto& [existingOrder, _] =  orders.at(order.GetOrderId());
			CancelOrder(order.GetOrderId());
			return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
		}

		std::size_t Size() const { return orders.size(); }

		OrderbookLevelInfos GetOrderInfos() const{

			// reserve makes only 1 malloc call
			// 	could optimize this as not all orders are bids nor asks?
			LevelInfos bidInfos, askInfos;
			bidInfos.reserve(orders.size());
			askInfos.reserve(orders.size());

			auto CreateLevelInfos = [](Price price, const OrderPointers& orders){
				return LevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity) 0, 
						[ ](std::size_t runningSum, const OrderPointer& order){
							return runningSum + order->GetRemainingQuantity(); })
						};
			};

			for(const auto& [price, orders] : bids)
				bidInfos.push_back(CreateLevelInfos(price, orders));

			for(const auto& [price, orders] : asks)
				askInfos.push_back(CreateLevelInfos(price, orders));

			return OrderbookLevelInfos{bidInfos, askInfos};

		}

};



int main()
{

	// quick test
	Orderbook orderbook;
	const OrderId orderId = 1;
	orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10)); 
	
	std::cout <<orderbook.Size()<< std::endl;
	orderbook.CancelOrder(orderId);
	std::cout <<orderbook.Size()<< std::endl;

	return 0;
}

