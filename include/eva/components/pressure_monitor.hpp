#pragma once

#include <any>

#include "eva/message_bus.hpp"

namespace eva {

// Placeholder component: subscribes to raw suit pressure telemetry and
// publishes an alert when the value falls outside a safe range.
class PressureMonitor {
public:
    explicit PressureMonitor(MessageBus& bus);

private:
    void onTelemetry(const std::any& payload);

    MessageBus& bus_;

    static constexpr double kMinSafePressureKpa = 29.6;
    static constexpr double kMaxSafePressureKpa = 34.5;
};

} // namespace eva
