#include "Orderbook.hpp"

#include <algorithm>
#include <numeric>

// order came in, can I match the request
bool Orderbook::CanMatch(Side side, Price price) const
{
	// order that came in is a Buy
	if (side == Side::Buy) {
		// we have no asks
		if (asks.empty()) return false;

		// we choose asks with the lowest price
		// taking just the value of the bestAsk
		const auto& [bestAsk, _] = *asks.begin();
		return price >= bestAsk;
	}
	// order that came in is a Sell
	else {
		if (bids.empty()) return false;

		const auto& [bestBid, _] = *bids.begin();
		return price <= bestBid;
	}
}

Trades Orderbook::MatchOrders()
{
	Trades trades;
	// reserve in memory?
	trades.reserve(orders.size());

	while (true) {
		if (bids.empty() || asks.empty()) break;

		auto& [bidPrice, bidsAtbidPrice] = *this->bids.begin();
		auto& [askPrice, asksAtaskPrice] = *this->asks.begin();

		if (bidPrice < askPrice) break;

		while (bidsAtbidPrice.size() && asksAtaskPrice.size()) {

			auto& bid = bidsAtbidPrice.front();
			auto& ask = asksAtaskPrice.front();

			Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());
			bid->Fill(quantity);
			ask->Fill(quantity);

			// make the trade
			trades.push_back(Trade{
					TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity}, 
					TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}});

			if (bid->IsFilled()) {
				// take the bid off the OrderPointers list and destroy the order pointer
				bidsAtbidPrice.pop_front();
				orders.erase(bid->GetOrderId());
			}
			
			if (ask->IsFilled()) {
				// take the ask off the OrderPointers list and destroy the order pointer
				asksAtaskPrice.pop_front();
				orders.erase(ask->GetOrderId());
			}

			// if I have no more bids/asks at the price level, then eliminate the price level
			if (bidsAtbidPrice.empty()) bids.erase(bidPrice);
			
			if (asksAtaskPrice.empty()) asks.erase(askPrice);

		}

	}

	// we need to handle FillandKill orders
	// To note:
	// 	We throw away FAK orders that don't cross (match the other order type)
	if (!bids.empty()) {
		auto& [_, bids] = *this->bids.begin();
		auto& order = bids.front();
		if (order->GetOrderType() == OrderType::FillAndKill)
			CancelOrder(order->GetOrderId());
	}
			
	if (!asks.empty()) {
		auto& [_, asks] = *this->asks.begin();
		auto& order = asks.front();
		if (order->GetOrderType() == OrderType::FillAndKill)
			CancelOrder(order->GetOrderId());
	}

	return trades;
}

Trades Orderbook::AddOrder(OrderPointer newOrder)
{
	if (orders.contains(newOrder->GetOrderId()))
		return {};

	// handle FillAndKill orders that don't match any prices
	if (newOrder->GetOrderType() == OrderType::FillAndKill && !CanMatch(newOrder->GetSide(), newOrder->GetPrice()))
		return {};
	
	// we're storing this iterator that points to the orders list for O(1) order cancellation
	OrderPointers::iterator iterator;
	
	// we want to get the pointer to the list at the order's price level
	// I want to insert 

	if (newOrder->GetSide() == Side::Buy) {
		auto& orderList = this->bids[newOrder->GetPrice()];
		orderList.push_back(newOrder);
		iterator = std::next(orderList.begin(), orderList.size() - 1);
	}
	else {
		auto& orderList = this->asks[newOrder->GetPrice()];
		orderList.push_back(newOrder);
		iterator = std::next(orderList.begin(), orderList.size() - 1);
	}
	
	orders.insert({newOrder->GetOrderId(), OrderEntry{newOrder, iterator}});
	return MatchOrders();
}

void Orderbook::CancelOrder(OrderId orderId)
{
	if (!orders.contains(orderId)) return;

	const auto& [order, orderIterator] = orders.at(orderId);
	
	if (order->GetSide() == Side::Sell) {
		auto price = order->GetPrice();
		// list of asks at a certain price, i want to take the orderId order off it
		auto& orderList = asks.at(price);
		orderList.erase(orderIterator);
		if (orderList.empty())
			asks.erase(price);
	}
	else {
		auto price = order->GetPrice();
		// list of bids at a certain price, i want to take the orderId order off it
		auto& orderList = bids.at(price);
		orderList.erase(orderIterator);
		if (orderList.empty())
			bids.erase(price);
	}

	orders.erase(orderId);
}

Trades Orderbook::MatchOrder(OrderModify order)
{
	if (!orders.contains(order.GetOrderId()))
		return {};

	const auto& [existingOrder, _] = orders.at(order.GetOrderId());
	CancelOrder(order.GetOrderId());
	return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
}

std::size_t Orderbook::Size() const { return orders.size(); }

OrderbookLevelInfos Orderbook::GetOrderInfos() const
{
	// reserve makes only 1 malloc call
	// 	could optimize this as not all orders are bids nor asks?
	LevelInfos bidInfos, askInfos;
	bidInfos.reserve(orders.size());
	askInfos.reserve(orders.size());

	auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
		return LevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity) 0, 
				[](std::size_t runningSum, const OrderPointer& order) {
					return runningSum + order->GetRemainingQuantity(); })
				};
	};

	for (const auto& [price, orders] : bids)
		bidInfos.push_back(CreateLevelInfos(price, orders));

	for (const auto& [price, orders] : asks)
		askInfos.push_back(CreateLevelInfos(price, orders));

	return OrderbookLevelInfos{bidInfos, askInfos};
}
