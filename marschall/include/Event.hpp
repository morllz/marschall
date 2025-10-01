#pragma once
#include <type_traits>


/// Unique key type for each event type
using EventTypeKey = const void*;

/*
 * Abstract base class all events must inherit from.
 *
 */
class Event
{
public:
	virtual ~Event() = default;
};


/// Concept to ensure a type is derived from Event
template<class T>
concept EventType = std::is_convertible_v<T*, Event*>;

/*
 * Base class for events to inherit from.
 * 
 * Implements the getTypeKey method using CRTP to provide a unique type key for each event type.
 * 
 * @tparam EType The event type.
 */