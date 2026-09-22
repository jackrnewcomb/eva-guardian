#include "eva/earth_relay.hpp"

#include <iostream>
#include <utility>

namespace eva {

EarthRelay::EarthRelay(MessageBus& bus, std::chrono::milliseconds roundTripDelay)
    : roundTripDelay_(roundTripDelay) {
    bus.subscribe("alerts", [this](const std::any& payload) { onAlert(payload); });

    running_ = true;
    worker_ = std::thread(&EarthRelay::relayLoop, this);
}

EarthRelay::~EarthRelay() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        running_ = false;
    }
    queueCv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void EarthRelay::onAlert(const std::any& payload) {
    const auto& alert = std::any_cast<const Alert&>(payload);
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push(QueuedAlert{alert, std::chrono::steady_clock::now()});
    }
    queueCv_.notify_one();
}

void EarthRelay::relayLoop() {
    while (true) {
        QueuedAlert item;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] { return !queue_.empty() || !running_; });

            if (!running_ && queue_.empty()) {
                return;
            }

            item = std::move(queue_.front());
            queue_.pop();
        }

        std::this_thread::sleep_until(item.receivedAt + roundTripDelay_);

        std::cout << "[EARTH RELAY, +" << roundTripDelay_.count() << "ms delay] " << item.alert.source
                   << " (" << item.alert.suitId << "): " << item.alert.message << " -> "
                   << item.alert.recommendedAction << "\n";
    }
}

} // namespace eva
