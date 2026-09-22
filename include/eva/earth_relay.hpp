#pragma once

#include <any>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"

namespace eva {

// Simulates the naive alternative this product replaces: relaying alerts to
// Earth and waiting for Mission Control to react. Reacts to the same
// "alerts" the local monitors publish, but only after a simulated light-delay
// round trip, so it can be contrasted against the instant local alert on the
// same bus. Runs its own worker thread so the delay never blocks the bus.
class EarthRelay {
public:
    EarthRelay(MessageBus& bus, std::chrono::milliseconds roundTripDelay);
    ~EarthRelay();

    EarthRelay(const EarthRelay&) = delete;
    EarthRelay& operator=(const EarthRelay&) = delete;

private:
    struct QueuedAlert {
        Alert alert;
        std::chrono::steady_clock::time_point receivedAt;
    };

    void onAlert(const std::any& payload);
    void relayLoop();

    std::chrono::milliseconds roundTripDelay_;

    std::queue<QueuedAlert> queue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;

    std::thread worker_;
    bool running_ = false;
};

} // namespace eva
