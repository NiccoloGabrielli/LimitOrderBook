#include "OrderbookLevelInfos.hpp"

OrderbookLevelInfos::OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
	: bids{bids}, asks{asks}
{
}

const LevelInfos& OrderbookLevelInfos::GetBids() const
{
	return bids;
}

const LevelInfos& OrderbookLevelInfos::GetAsks() const
{
	return asks;
}
