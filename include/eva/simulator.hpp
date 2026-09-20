#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

// Periodically publishes telemetry for a single suit. Values default to
// nominal and can be overridden live via handleCommand() (e.g. from stdin)
// to inject anomalies during a demo.
class Simulator {
public:
    Simulator(MessageBus& bus, std::string suitId);
    ~Simulator();

    void start();
    void stop();

    // Parses one command line: "o2 <value>", "pressure <value>",
    // "thermal <value>", or "reset". Returns false for "quit"/"exit".
    bool handleCommand(const std::string& line);

private:
    void publishLoop();
    void publishFrame();

    MessageBus& bus_;
    std::string suitId_;

    std::atomic<double> o2_;
    std::atomic<double> pressure_;
    std::atomic<double> thermal_;

    std::thread worker_;
    std::atomic<bool> running_{false};

    static constexpr double kNominalO2 = 20.9;
    static constexpr double kNominalPressure = 32.0;
    static constexpr double kNominalThermal = 24.0;
};

} // namespace eva
