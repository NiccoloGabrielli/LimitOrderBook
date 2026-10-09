#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>


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
		Order(OrderType orderType, OrderId, orderId, Side side, Price price, Quantity quantity): orderType{orderType}, orderId{orderId}, side{side}, price{price}, quantity{quantity} {}

	private:
		OrderType orderType;
		OrderId orderId;
		Side side;
		Price price;
		Quantity quantity;
};

// i'm not going to alias the OrderPointer or OrderPointers


int main()
{
	return 0;
}

