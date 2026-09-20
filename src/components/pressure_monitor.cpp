#include "eva/components/pressure_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

PressureMonitor::PressureMonitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void PressureMonitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    AlertSeverity level = AlertSeverity::Info;
    if (frame.pressureKpa < kCriticalMinPressureKpa || frame.pressureKpa > kCriticalMaxPressureKpa) {
        level = AlertSeverity::Critical;
    } else if (frame.pressureKpa < kWarnMinPressureKpa || frame.pressureKpa > kWarnMaxPressureKpa) {
        level = AlertSeverity::Warning;
    }

    if (level == lastLevel_) {
        return;
    }
    lastLevel_ = level;

    const std::string message =
        level == AlertSeverity::Info ? "Suit pressure back to nominal" : "Suit pressure outside safe range";
    bus_.publish("alerts", Alert{"PressureMonitor", frame.suitId, level, message, frame});
}

} // namespace eva
