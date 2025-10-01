#pragma once
#include "EventListener.hpp"
#include <memory>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <queue>

/*
 * EventDispatcher manages event subscriptions and dispatching.
 * 
 * Listeners can subscribe to specific event types, and events can be dispatched to notify those listeners.
 * Supports one-time subscriptions and automatic removal of expired listeners.
 */
class EventDispatcher
{
public:

	/*
	 * Subscribe a listener to a specific event type.
	 *
	 * @tparam EType The event type to subscribe to.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is stored as a weak pointer to avoid dangling references.
	 */
	template <EventType EType>
	void subscribeTo(const std::shared_ptr<EventListener<EType>>& listener)
	{
		auto& subs = subscriptions[typeid(EType).hash_code()];

		std::weak_ptr<EventListener<EType>> weak = listener;

		const IEventListener* id = listener.get();

		subs.emplace(Subscriber{
			id,
			[weak = std::move(weak)](const Event& event)
			{
				if (auto l = weak.lock())
					l->onEvent(static_cast<const EType&>(event));
				else
					return false;
				return true;
			}
			});
	}

	/*
	 * Subscribe a listener to multiple event types.
	 *
	 * @tparam EType The event types to subscribe to.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is stored as a weak pointer to avoid dangling references.
	 */
	template <EventType... EType, typename T>
		requires (sizeof...(EType) > 1)
	void subscribeTo(const std::shared_ptr<T>& listener)
	{
		(subscribeTo<EType>(listener), ...);
	}

	/*
	 * Subscribe a listener to a specific event type for a single notification.
	 *
	 * @tparam EType The event type to subscribe to.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is stored as a weak pointer to avoid dangling references.
	 *          After the first event is received, the listener is automatically unsubscribed.
	 */
	template <EventType EType>
	void subscribeOnceTo(const std::shared_ptr<EventListener<EType>>& listener)
	{
		auto& subs = subscriptions[typeid(EType).hash_code()];

		std::weak_ptr<EventListener<EType>> weak = listener;

		const IEventListener* id = listener.get();

		subs.emplace(Subscriber{
			id,
			[weak = std::move(weak)](const Event& event)
			{
				if (auto l = weak.lock())
					l->onEvent(static_cast<const EType&>(event));
				return false;
			}
			});
	}

	/*
	 * Subscribe a listener to multiple event types for a single notification each.
	 *
	 * @tparam EType The event types to subscribe to.
	 * @template T The type of the listener.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is stored as a weak pointer to avoid dangling references.
	 *          After the first event of each type is received, the listener is automatically unsubscribed from that type.
	 */
	template <EventType... EType, typename T>
		requires (sizeof...(EType) > 1)
	void subscribeOnceTo(const std::shared_ptr<T>& listener)
	{
		(subscribeOnceTo<EType>(listener), ...);
	}

	/*
	 * Unsubscribe a listener from a specific event type.
	 *
	 * @tparam EType The event type to unsubscribe from.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is identified by its pointer address.
	 */
	template <EventType EType>
	void unsubscribeFrom(const std::shared_ptr<EventListener<EType>>& listener)
	{
		unsubscribeFrom<EType>(listener.get());
	}

	/*
	 * Unsubscribe a listener from multiple event types.
	 *
	 * @tparam EType The event types to unsubscribe from.
	 * @template T The type of the listener.
	 * @param listener A shared pointer to the listener.
	 *
	 * @remarks The listener is identified by its pointer address.
	 */
	template <EventType... EType, typename T>
	void unsubscribeFrom(const std::shared_ptr<EventListener<T>>& listener)
	{
		(unsubscribeFrom<EType>(listener.get()), ...);
	}

	/*
	 * Dispatch an event to all subscribed listeners.
	 *
	 * This blocks until all listeners have been notified.
	 *
	 * @param event The event to dispatch.
	 * 
	 */
	void dispatch(const Event& event)
	{
		auto it = subscriptions.find(typeid(event).hash_code());
		if (it != subscriptions.end())
		{
			std::erase_if(it->second, [&event](const Subscriber& s) {
				return !s.callback(event);
				});
		}
	}
	/*
	 * Dispatch a specific event type to all subscribed listeners.
	 *
	 * This blocks until all listeners have been notified.
	 *
	 *	Use this if you have a specific event type and want to avoid the overhead of virtual dispatch.
	 * 
	 * @tparam EType The event type to dispatch.
	 * @param event The event to dispatch.
	 * 
	 */
	template <EventType EType>
		requires !std::is_same_v<EType, Event>
	void dispatch(const EType& event)
	{
		auto it = subscriptions.find(typeid(event).hash_code());
		if (it != subscriptions.end())
		{
			std::erase_if(it->second, [&event](const Subscriber& s) {
				return !s.callback(event);
				});
		}
	}

	/*
	 * Queue an event for later processing.
	 *
	 * The event will be processed when processQueue is called.
	 *
	 * @param event A unique pointer to the event to queue.
	 */
	void queueEvent(std::unique_ptr<Event> event)
	{
		eventQueue.push(std::move(event));
	}

	/*
	 * Process all queued events, dispatching them to their subscribed listeners.
	 *
	 * This blocks until all queued events have been processed.
	 */
	void processQueue()
	{
		while (!eventQueue.empty())
		{
			dispatch(*eventQueue.front());
			eventQueue.pop();
		}
	}

private:
	template <EventType EType>
	void unsubscribeFrom(const EventListener<EType>* id)
	{
		auto& subs = subscriptions[typeid(EType).hash_code()];

		subs.erase(static_cast<const IEventListener*>(id));
	}

	using Callback = std::function<bool(const Event&)>;

	struct Subscriber
	{
		const IEventListener* id;
		Callback callback;
	};

	struct SubscriberHash {
		using is_transparent = void;
		std::size_t operator()(const Subscriber& s) const noexcept {
			return std::hash<const IEventListener*>{}(s.id);
		}
		std::size_t operator()(const IEventListener* id) const noexcept {
			return std::hash<const IEventListener*>{}(id);
		}
	};
	struct SubscriberEqual {
		using is_transparent = void;
		bool operator()(const Subscriber& lhs, const Subscriber& rhs) const noexcept {
			return lhs.id == rhs.id;
		}
		bool operator()(const Subscriber& lhs, const IEventListener* rhs) const noexcept {
			return lhs.id == rhs;
		}
		bool operator()(const IEventListener* lhs, const Subscriber& rhs) const noexcept {
			return lhs == rhs.id;
		}
	};

	std::unordered_map<size_t, std::unordered_set<Subscriber, SubscriberHash, SubscriberEqual>> subscriptions;
	std::queue<std::unique_ptr<Event>> eventQueue;
};
