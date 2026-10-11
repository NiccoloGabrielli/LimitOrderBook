#pragma once

#include "Usings.hpp"
#include <vector>

// what do we do when an order is matched?
struct TradeInfo
{
	OrderId orderId;
	Price price;
	Quantity quantity;
};

class Trade
{
public:
	Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade);

	const TradeInfo& GetBidTrade() const;
	const TradeInfo& GetAskTrade() const;

private:
	TradeInfo bidTrade;
	TradeInfo askTrade;
};

using Trades = std::vector<Trade>;
