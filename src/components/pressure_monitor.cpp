#include "eva/components/pressure_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"
#include "eva/thresholds.hpp"

namespace eva {

PressureMonitor::PressureMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void PressureMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    AlertSeverity level = AlertSeverity::Info;
    if (frame.pressureKpa < kPressureCriticalMinKpa || frame.pressureKpa > kPressureCriticalMaxKpa) {
        level = AlertSeverity::Critical;
    } else if (frame.pressureKpa < kPressureWarnMinKpa || frame.pressureKpa > kPressureWarnMaxKpa) {
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
            message = "Suit pressure outside safe range";
            action = "Abort EVA immediately: check suit seals/valves and return to airlock.";
            break;
        case AlertSeverity::Warning:
            message = "Suit pressure outside safe range";
            action = "Monitor suit pressure closely; inspect seals at next opportunity.";
            break;
        case AlertSeverity::Info:
            message = "Suit pressure back to nominal";
            action = "No action needed.";
            break;
    }

    bus_.publish("alerts", Alert{"PressureMonitor", frame.suitId, "Pressure", level, message, action, frame});
}

} // namespace eva
