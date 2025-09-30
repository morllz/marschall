#pragma once
#include "EventListener.hpp"
#include <memory>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <queue>

class EventDispatcher
{
public:

	template <EventType EType>
	void subscribeTo(const std::shared_ptr<EventListener<EType>>& listener)
	{
		auto& subs = subscriptions[getEventTypeKey<EType>()];

		std::weak_ptr<EventListener<EType>> weak = listener;

		const EventListener<EType>* id = listener.get();

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

	template <EventType... EType, typename T>
		requires (sizeof...(EType) > 1)
	void subscribeTo(const std::shared_ptr<T>& listener)
	{
		(subscribeTo<EType>(listener), ...);
	}

	template <EventType EType>
	void subscribeOnceTo(const std::shared_ptr<EventListener<EType>>& listener)
	{
		auto& subs = subscriptions[getEventTypeKey<EType>()];

		std::weak_ptr<EventListener<EType>> weak = listener;

		const EventListener<EType>* id = listener.get();

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

	template <EventType... EType, typename T>
		requires (sizeof...(EType) > 1)
	void subscribeOnceTo(const std::shared_ptr<T>& listener)
	{
		(subscribeOnceTo<EType>(listener), ...);
	}

	template <EventType EType>
	void unsubscribeFrom(const EventListener<EType>* id)
	{
		auto& subs = subscriptions[getEventTypeKey<EType>()];

		subs.erase(static_cast<const void*>(id));
	}

	template <EventType EType>
	void unsubscribeFrom(const std::shared_ptr<EventListener<EType>>& listener)
	{
		unsubscribeFrom<EType>(listener.get());
	}

	void dispatch(const Event& event)
	{
		auto it = subscriptions.find(event.getTypeKey());
		if (it != subscriptions.end())
		{
			std::erase_if(it->second, [&event](const Subscriber& s) {
				return !s.callback(event);
				});
		}
	}
	template <EventType EType>
		requires !std::is_same_v<EType, Event>
	void dispatch(const EType& event)
	{
		auto it = subscriptions.find(getEventTypeKey<EType>());
		if (it != subscriptions.end())
		{
			std::erase_if(it->second, [&event](const Subscriber& s) {
				return !s.callback(event);
				});
		}
	}

	void queueEvent(std::unique_ptr<Event> event)
	{
		eventQueue.push(std::move(event));
	}

	void processQueue()
	{
		while (!eventQueue.empty())
		{
			dispatch(*eventQueue.front());
			eventQueue.pop();
		}
	}

private:
	using Callback = std::function<bool(const Event&)>;

	struct Subscriber
	{
		const void* id;
		Callback callback;
	};

	struct SubscriberHash {
		using is_transparent = void;
		std::size_t operator()(const Subscriber& s) const noexcept {
			return std::hash<const void*>{}(s.id);
		}
		std::size_t operator()(const void* id) const noexcept {
			return std::hash<const void*>{}(id);
		}
	};
	struct SubscriberEqual {
		using is_transparent = void;
		bool operator()(const Subscriber& lhs, const Subscriber& rhs) const noexcept {
			return lhs.id == rhs.id;
		}
		bool operator()(const Subscriber& lhs, const void* rhs) const noexcept {
			return lhs.id == rhs;
		}
		bool operator()(const void* lhs, const Subscriber& rhs) const noexcept {
			return lhs == rhs.id;
		}
	};

	std::unordered_map<EventTypeKey, std::unordered_set<Subscriber, SubscriberHash, SubscriberEqual>> subscriptions;
	std::queue<std::unique_ptr<Event>> eventQueue;
};
