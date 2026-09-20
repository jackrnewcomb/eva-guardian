#include "eva/components/thermal_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

ThermalMonitor::ThermalMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void ThermalMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    if (frame.thermalCelsius < kMinSafeTempCelsius || frame.thermalCelsius > kMaxSafeTempCelsius) {
        bus_.publish("alerts", Alert{
            "ThermalMonitor",
            frame.suitId,
            AlertSeverity::Critical,
            "Suit temperature outside safe range",
            frame,
        });
    }
}

} // namespace eva
