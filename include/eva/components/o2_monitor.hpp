#pragma once

#include <any>

#include "eva/message_bus.hpp"

namespace eva {

// Placeholder component: subscribes to raw O2 telemetry and publishes an
// alert when the value falls outside a safe range.
class O2Monitor {
public:
    explicit O2Monitor(MessageBus& bus);

private:
    void onTelemetry(const std::any& payload);

    MessageBus& bus_;

    static constexpr double kMinSafeO2Percent = 19.5;
};

} // namespace eva
