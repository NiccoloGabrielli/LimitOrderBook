#pragma once

// scoped enumeration (C++ 11), need to use the OrderType::... to access elements
enum class OrderType
{
	// order remains active indefinitely until its completely filled, or explicitly cancelled by the user
	GoodTillCancel,
	
	// Filled upon arrival, whatever quantity cannot be filled immediately is killed on the spot
	FillAndKill
};
