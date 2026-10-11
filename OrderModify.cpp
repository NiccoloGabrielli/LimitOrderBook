#include "OrderModify.hpp"

OrderModify::OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
	: orderId{orderId}, 
	  price{price},
	  quantity{quantity}
{
}

OrderId OrderModify::GetOrderId() const { return orderId; }
Price OrderModify::GetPrice() const { return price; }
Side OrderModify::GetSide() const { return side; }
Quantity OrderModify::GetQuantity() const { return quantity; }

OrderPointer OrderModify::ToOrderPointer(OrderType type) const
{
	return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
}
