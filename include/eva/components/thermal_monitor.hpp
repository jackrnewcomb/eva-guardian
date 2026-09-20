#pragma once

#include <any>

#include "eva/message_bus.hpp"

namespace eva {

// Placeholder component: subscribes to raw thermal telemetry and publishes
// an alert when the value falls outside a safe range.
class ThermalMonitor {
public:
    explicit ThermalMonitor(MessageBus& bus);

private:
    void onTelemetry(const std::any& payload);

    MessageBus& bus_;

    static constexpr double kMinSafeTempCelsius = 18.0;
    static constexpr double kMaxSafeTempCelsius = 32.0;
};

} // namespace eva
