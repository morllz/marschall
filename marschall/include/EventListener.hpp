#pragma once
#include "Event.hpp"

class IEventListener
{
public:
	virtual ~IEventListener() = default;
};

template<EventType EType>
class EventListener : public IEventListener
{
public:
	virtual ~EventListener() = default;
	virtual void onEvent(const EType& event) = 0;
};

template <EventType... EType>
class MultiEventListener : public EventListener<EType>...
{
	public:
	virtual ~MultiEventListener() = default;
};