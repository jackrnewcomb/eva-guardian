#include "eva/components/pressure_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

PressureMonitor::PressureMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void PressureMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    if (frame.pressureKpa < kMinSafePressureKpa || frame.pressureKpa > kMaxSafePressureKpa) {
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
