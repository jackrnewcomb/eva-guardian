#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"

namespace eva {

// Placeholder component: subscribes to suit telemetry and publishes an
// alert only when a suit's thermal reading crosses into/out of a warning or
// critical band, rather than on every tick it stays out of range.
class ThermalMonitor {
public:
    explicit ThermalMonitor(MessageBus& bus);

private:
    void onTelemetry(const std::any& payload);

    MessageBus& bus_;
    std::unordered_map<std::string, AlertSeverity> lastLevelBySuit_;
};

} // namespace eva
