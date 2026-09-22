#include "test_harness.hpp"

#include "eva/message_bus.hpp"

#include <any>
#include <atomic>
#include <chrono>
#include <thread>

namespace {

// The bus dispatches on its own worker thread, so tests poll briefly instead
// of assuming delivery is synchronous.
template <typename Predicate>
bool waitUntil(Predicate predicate) {
    for (int i = 0; i < 200; ++i) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return predicate();
}

} // namespace

TEST_CASE(MessageBusDeliversToSubscriber) {
    eva::MessageBus bus;
    std::atomic<int> received{0};
    std::atomic<int> lastValue{0};

    bus.subscribe("topic.test", [&](const std::any& payload) {
        lastValue = std::any_cast<int>(payload);
        received++;
    });

    bus.publish("topic.test", 42);

    CHECK(waitUntil([&] { return received.load() == 1; }));
    CHECK(lastValue == 42);
}

TEST_CASE(MessageBusOnlyDeliversToMatchingTopic) {
    eva::MessageBus bus;
    std::atomic<int> receivedA{0};
    std::atomic<int> receivedB{0};

    bus.subscribe("topic.a", [&](const std::any&) { receivedA++; });
    bus.subscribe("topic.b", [&](const std::any&) { receivedB++; });

    bus.publish("topic.a", 1);

    CHECK(waitUntil([&] { return receivedA.load() == 1; }));
    CHECK(receivedB == 0);
}
