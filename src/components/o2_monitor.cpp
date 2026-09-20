#include "eva/components/o2_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

O2Monitor::O2Monitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void O2Monitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    AlertSeverity level = AlertSeverity::Info;
    if (frame.o2Percent < kCriticalO2Percent) {
        level = AlertSeverity::Critical;
    } else if (frame.o2Percent < kWarnO2Percent) {
        level = AlertSeverity::Warning;
    }

    if (level == lastLevel_) {
        return;
    }
    lastLevel_ = level;

    const std::string message =
        level == AlertSeverity::Info ? "O2 level back to nominal" : "O2 level below safe threshold";
    bus_.publish("alerts", Alert{"O2Monitor", frame.suitId, level, message, frame});
}

} // namespace eva
