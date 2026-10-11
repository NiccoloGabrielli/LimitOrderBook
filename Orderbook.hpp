#pragma once

#include "Order.hpp"
#include "OrderModify.hpp"
#include "OrderbookLevelInfos.hpp"
#include "Trade.hpp"

#include <map>
#include <unordered_map>

// now I actually need to make the orderbook
// To note:
// how do we store our maps, maps or unorderped maps
// we're going to use map to store our bids, because we can sort the bids
// order and location we will represent with an orderentry obj

class Orderbook
{
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
	bool CanMatch(Side side, Price price) const;

	Trades MatchOrders();

public:
	Trades AddOrder(OrderPointer newOrder);
	void CancelOrder(OrderId orderId);
	Trades MatchOrder(OrderModify order);
	std::size_t Size() const;
	OrderbookLevelInfos GetOrderInfos() const;
};
