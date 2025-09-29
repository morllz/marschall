#include <gtest/gtest.h>
#include <memory>
#include "EventDispatcher.h"
#include "EventListener.h"
#include "Event.h"

// Test event types
class TestEventA : public Event {};
class TestEventB : public Event {};

// Simple listener for TestEventA
class TestListenerA : public EventListener<TestEventA> {
public:
    int callCount = 0;
    void onEvent(const TestEventA&) override { ++callCount; }
};

// Simple listener for TestEventB
class TestListenerB : public EventListener<TestEventB> {
public:
    int callCount = 0;
    void onEvent(const TestEventB&) override { ++callCount; }
};

// Multi-event listener
class TestMultiListener : public MultiEventListener<TestEventA, TestEventB> {
public:
    int aCount = 0;
    int bCount = 0;
    void onEvent(const TestEventA&) override { ++aCount; }
    void onEvent(const TestEventB&) override { ++bCount; }
};

TEST(EventDispatcher, SubscribeAndDispatch) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeTo<TestEventA>(listener);

    TestEventA event;
    dispatcher.dispatch(event);

    EXPECT_EQ(listener->callCount, 1);
}

TEST(EventDispatcher, Unsubscribe) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeTo<TestEventA>(listener);

    dispatcher.unsubscribeFrom<TestEventA>(listener);

    TestEventA event;
    dispatcher.dispatch(event);

    EXPECT_EQ(listener->callCount, 0);
}

TEST(EventDispatcher, SubscribeOnce) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeOnceTo<TestEventA>(listener);

    TestEventA event;
    dispatcher.dispatch(event);
    dispatcher.dispatch(event);

    EXPECT_EQ(listener->callCount, 1);
}

TEST(EventDispatcher, MultiEventListener) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestMultiListener>();
    dispatcher.subscribeTo<TestEventA, TestEventB>(listener);

    TestEventA eventA;
    TestEventB eventB;
    dispatcher.dispatch(eventA);
    dispatcher.dispatch(eventB);

    EXPECT_EQ(listener->aCount, 1);
    EXPECT_EQ(listener->bCount, 1);
}

TEST(EventDispatcher, SubscribeOnceMultiEvent) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestMultiListener>();
    dispatcher.subscribeOnceTo<TestEventA>(listener);
    dispatcher.subscribeOnceTo<TestEventB>(listener);

    TestEventA eventA;
    TestEventB eventB;
    dispatcher.dispatch(eventA);
    dispatcher.dispatch(eventA);
    dispatcher.dispatch(eventB);
    dispatcher.dispatch(eventB);

    EXPECT_EQ(listener->aCount, 1);
    EXPECT_EQ(listener->bCount, 1);
}

TEST(EventDispatcher, UnsubscribeByPointer) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeTo<TestEventA>(listener);

    dispatcher.unsubscribeFrom(listener.get());

    TestEventA event;
    dispatcher.dispatch(event);

    EXPECT_EQ(listener->callCount, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}