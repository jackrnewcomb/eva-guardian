#pragma once

#include <any>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace eva {

// Simple in-process async pub/sub bus: publish() enqueues, a worker thread
// dispatches to subscribers so callers never block on subscriber logic.
class MessageBus {
public:
    using Callback = std::function<void(const std::any&)>;

    MessageBus();
    ~MessageBus();

    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    void subscribe(const std::string& topic, Callback callback);
    void publish(const std::string& topic, std::any payload);

    void start();
    void stop();

private:
    struct Message {
        std::string topic;
        std::any payload;
    };

    void dispatchLoop();

    std::unordered_map<std::string, std::vector<Callback>> subscribers_;
    std::mutex subscribersMutex_;

    std::queue<Message> queue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;

    std::thread worker_;
    bool running_ = false;
};

} // namespace eva
