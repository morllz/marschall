#include <gtest/gtest.h>
#include <memory>
#include "marschall.hpp"

class TestEventA : public Event {};
class TestEventB : public Event {};

class TestListenerA : public EventListener<TestEventA> {
public:
    int callCount = 0;
    void onEvent(const TestEventA&) override { ++callCount; }
};

class TestListenerB : public EventListener<TestEventB> {
public:
    int callCount = 0;
    void onEvent(const TestEventB&) override { ++callCount; }
};

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

TEST(EventDispatcher, RemoveExpiredSubscribers) {
    EventDispatcher dispatcher;
    int callCount = 0;

    {
        struct LocalListener : public EventListener<TestEventA> {
            int* callCountPtr;
            LocalListener(int* ptr) : callCountPtr(ptr) {}
            void onEvent(const TestEventA&) override { ++(*callCountPtr); }
        };
        auto listener = std::make_shared<LocalListener>(&callCount);
        dispatcher.subscribeTo<TestEventA>(listener);

        TestEventA event;
        dispatcher.dispatch(event);
        EXPECT_EQ(callCount, 1);
    }

    TestEventA event;
    dispatcher.dispatch(event);
    EXPECT_EQ(callCount, 1);
}

TEST(EventDispatcher, QueueEventAndProcessQueue) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeTo<TestEventA>(listener);

    TestEventA event;
    dispatcher.queueEvent(std::make_unique<TestEventA>(event));

    EXPECT_EQ(listener->callCount, 0);

    dispatcher.processQueue();

    EXPECT_EQ(listener->callCount, 1);
}

TEST(EventDispatcher, QueueMultipleEvents) {
    EventDispatcher dispatcher;
    auto listenerA = std::make_shared<TestListenerA>();
    auto listenerB = std::make_shared<TestListenerB>();
    dispatcher.subscribeTo<TestEventA>(listenerA);
    dispatcher.subscribeTo<TestEventB>(listenerB);

    TestEventA eventA;
    TestEventB eventB;
    dispatcher.queueEvent(std::make_unique<TestEventA>(eventA));
    dispatcher.queueEvent(std::make_unique<TestEventB>(eventB));

    EXPECT_EQ(listenerA->callCount, 0);
    EXPECT_EQ(listenerB->callCount, 0);

    dispatcher.processQueue();

    EXPECT_EQ(listenerA->callCount, 1);
    EXPECT_EQ(listenerB->callCount, 1);
}

TEST(EventDispatcher, QueueEventWithNoSubscribers) {
    EventDispatcher dispatcher;
    TestEventA event;
    dispatcher.queueEvent(std::make_unique<TestEventA>(event));

    dispatcher.processQueue();
}

TEST(EventDispatcher, QueueEventUnsubscribedBeforeProcess) {
    EventDispatcher dispatcher;
    auto listener = std::make_shared<TestListenerA>();
    dispatcher.subscribeTo<TestEventA>(listener);

    TestEventA event;
    dispatcher.queueEvent(std::make_unique<TestEventA>(event));

    dispatcher.unsubscribeFrom<TestEventA>(listener);

    dispatcher.processQueue();

    EXPECT_EQ(listener->callCount, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}