#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"

namespace eva {

// Placeholder component: subscribes to suit telemetry and publishes an
// alert only when a suit's O2 reading crosses into/out of a warning or
// critical band, rather than on every tick it stays out of range.
class O2Monitor {
public:
    explicit O2Monitor(MessageBus& bus);

private:
    void onTelemetry(const std::any& payload);

    MessageBus& bus_;
    std::unordered_map<std::string, AlertSeverity> lastLevelBySuit_;

    static constexpr double kWarnO2Percent = 19.5;
    static constexpr double kCriticalO2Percent = 18.0;
};

} // namespace eva
