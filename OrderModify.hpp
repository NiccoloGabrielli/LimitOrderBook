#pragma once

#include "Order.hpp"

// class for the API
class OrderModify
{
public:
	OrderModify(OrderId orderId, Side side, Price price, Quantity quantity);

	OrderId GetOrderId() const;
	Price GetPrice() const;
	Side GetSide() const;
	Quantity GetQuantity() const;

	OrderPointer ToOrderPointer(OrderType type) const;

private:
	OrderId orderId;
	Side side;
	Price price;
	Quantity quantity;
};
