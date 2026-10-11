#pragma once

#include "Usings.hpp"
#include <vector>

struct LevelInfo
{
	Price price;
	Quantity quantity;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{
public:
	OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks);

	const LevelInfos& GetBids() const; 
	const LevelInfos& GetAsks() const;

private:
	LevelInfos bids;
	LevelInfos asks;
};
