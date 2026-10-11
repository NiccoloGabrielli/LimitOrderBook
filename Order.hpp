#pragma once

#include "OrderType.hpp"
#include "Side.hpp"
#include "Usings.hpp"

#include <memory>
#include <list>

class Order
{
public:
	Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity);

	OrderId GetOrderId() const;
	Side GetSide() const;
	OrderType GetOrderType() const;
	Price GetPrice() const;
	Quantity GetInitialQuantity() const;
	Quantity GetRemainingQuantity() const;
	Quantity GetFilledQuantity() const;

	bool IsFilled() const;
	void Fill(Quantity quantity);
	void CancelOrder();

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
