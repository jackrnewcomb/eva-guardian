#include "eva/components/thermal_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

ThermalMonitor::ThermalMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void ThermalMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    AlertSeverity level = AlertSeverity::Info;
    if (frame.thermalCelsius < kCriticalMinTempCelsius || frame.thermalCelsius > kCriticalMaxTempCelsius) {
        level = AlertSeverity::Critical;
    } else if (frame.thermalCelsius < kWarnMinTempCelsius || frame.thermalCelsius > kWarnMaxTempCelsius) {
        level = AlertSeverity::Warning;
    }

    if (level == lastLevel_) {
        return;
    }
    lastLevel_ = level;

    const std::string message = level == AlertSeverity::Info ? "Suit temperature back to nominal"
                                                              : "Suit temperature outside safe range";
    bus_.publish("alerts", Alert{"ThermalMonitor", frame.suitId, level, message, frame});
}

} // namespace eva
