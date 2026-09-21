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

    AlertSeverity& lastLevel = lastLevelBySuit_[frame.suitId];
    if (level == lastLevel) {
        return;
    }
    lastLevel = level;

    std::string message;
    std::string action;
    switch (level) {
        case AlertSeverity::Critical:
            message = "Suit temperature outside safe range";
            action = "Abort EVA immediately: adjust suit thermal control and return to habitat.";
            break;
        case AlertSeverity::Warning:
            message = "Suit temperature outside safe range";
            action = "Monitor suit temperature; adjust cooling/heating settings.";
            break;
        case AlertSeverity::Info:
            message = "Suit temperature back to nominal";
            action = "No action needed.";
            break;
    }

    bus_.publish("alerts", Alert{"ThermalMonitor", frame.suitId, "Thermal", level, message, action, frame});
}

} // namespace eva
