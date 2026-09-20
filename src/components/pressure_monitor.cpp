#include "eva/components/pressure_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

PressureMonitor::PressureMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.pressure", [this](const std::any& payload) { onTelemetry(payload); });
}

void PressureMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const TelemetryFrame&>(payload);

    if (frame.value < kMinSafePressureKpa || frame.value > kMaxSafePressureKpa) {
        bus_.publish("alerts", Alert{
            "PressureMonitor",
            frame.suitId,
            AlertSeverity::Critical,
            "Suit pressure outside safe range",
            frame,
        });
    }
}

} // namespace eva
