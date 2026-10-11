#include "Order.hpp"
#include <stdexcept>

Order::Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
	: orderType{orderType}, 
	  orderId{orderId}, 
	  side{side}, 
	  price{price}, 
	  initialQuantity{quantity},
	  remainingQuantity{quantity}
{
}

OrderId Order::GetOrderId() const { return orderId; }
Side Order::GetSide() const { return side; }
OrderType Order::GetOrderType() const { return orderType; }
Price Order::GetPrice() const { return price; }
Quantity Order::GetInitialQuantity() const { return initialQuantity; }
Quantity Order::GetRemainingQuantity() const { return remainingQuantity; }
Quantity Order::GetFilledQuantity() const { return initialQuantity - remainingQuantity; }

bool Order::IsFilled() const { return remainingQuantity == 0; }

void Order::Fill(Quantity quantity)
{
	if (quantity <= remainingQuantity)
		remainingQuantity -= quantity;
	else
		throw std::logic_error("Can't overfill an order");
}

void Order::CancelOrder()
{
}
