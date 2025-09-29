#pragma once
#include <type_traits>

class Event
{
public:
	virtual ~Event() = default;
};

template<class T>
concept EventType = std::is_convertible_v<T*, Event*>;