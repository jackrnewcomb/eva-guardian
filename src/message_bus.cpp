#include "eva/message_bus.hpp"

#include <utility>

namespace eva {

MessageBus::MessageBus() { start(); }

MessageBus::~MessageBus() { stop(); }

void MessageBus::subscribe(const std::string& topic, Callback callback) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    subscribers_[topic].push_back(std::move(callback));
}

void MessageBus::publish(const std::string& topic, std::any payload) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push(Message{topic, std::move(payload)});
    }
    queueCv_.notify_one();
}

void MessageBus::start() {
    if (running_) {
        return;
    }
    running_ = true;
    worker_ = std::thread(&MessageBus::dispatchLoop, this);
}

void MessageBus::stop() {
    if (!running_) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        running_ = false;
    }
    queueCv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

void MessageBus::dispatchLoop() {
    while (true) {
        Message message;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] { return !queue_.empty() || !running_; });

            if (!running_ && queue_.empty()) {
                return;
            }

            message = std::move(queue_.front());
            queue_.pop();
        }

        std::vector<Callback> callbacks;
        {
            std::lock_guard<std::mutex> lock(subscribersMutex_);
            auto it = subscribers_.find(message.topic);
            if (it != subscribers_.end()) {
                callbacks = it->second;
            }
        }

        for (const auto& callback : callbacks) {
            callback(message.payload);
        }
    }
}

} // namespace eva
