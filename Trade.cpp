#include "Trade.hpp"

Trade::Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
	: bidTrade{bidTrade},
	  askTrade{askTrade} 
{
}

const TradeInfo& Trade::GetBidTrade() const { return bidTrade; }
const TradeInfo& Trade::GetAskTrade() const { return askTrade; }
