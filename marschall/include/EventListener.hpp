#pragma once
#include "Event.hpp"

/*
 * Abstract base class for event listeners.
 *
 * Listeners should inherit from EventListener<EType> for each event type they want to handle.
 */
class IEventListener
{
public:
	virtual ~IEventListener() = default;
};

/*
 * Template for event listeners for a specific event type.
 *
 * Inherit from EventListener<EType> to handle events of type EType.
 * When receiving an event, the onEvent method will be called.
 *
 * @tparam EType The event type this listener handles.
 */
template<EventType EType>
class EventListener : public IEventListener
{
public:
	virtual ~EventListener() = default;
	virtual void onEvent(const EType& event) = 0;
};

/*
 * Template for event listeners that handle multiple event types.
 *
 * Inherit from MultiEventListener<EType1, EType2, ...> to handle multiple event types.
 *
 * @tparam EType The event types this listener handles.
 */
template <EventType... EType>
class MultiEventListener : public EventListener<EType>...
{
	public:
	virtual ~MultiEventListener() = default;
};