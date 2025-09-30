#pragma once
#include <type_traits>

using EventTypeKey = const void*;

class Event
{
public:
	virtual ~Event() = default;
	virtual EventTypeKey getTypeKey() const = 0;
};

template<class T>
concept EventType = std::is_convertible_v<T*, Event*>;

template <EventType EType>
EventTypeKey getEventTypeKey() {
	static int key;
	return &key;
}

template <typename EType>
class EventBase : public Event
{
public:
	static EventTypeKey getStaticTypeKey()
	{
		return getEventTypeKey<EType>();
	}
	EventTypeKey getTypeKey() const override
	{
		return getStaticTypeKey();
	}
};