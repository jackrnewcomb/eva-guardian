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

    AlertSeverity& lastLevel = lastLevelBySuit_[frame.suitId];
    if (level == lastLevel) {
        return;
    }
    lastLevel = level;

    std::string message;
    std::string action;
    switch (level) {
        case AlertSeverity::Critical:
            message = "O2 level below safe threshold";
            action = "Abort EVA immediately: return to airlock and switch to backup O2 supply.";
            break;
        case AlertSeverity::Warning:
            message = "O2 level below safe threshold";
            action = "Monitor O2 closely; prepare to shorten EVA duration.";
            break;
        case AlertSeverity::Info:
            message = "O2 level back to nominal";
            action = "No action needed.";
            break;
    }

    bus_.publish("alerts", Alert{"O2Monitor", frame.suitId, "O2", level, message, action, frame});
}

} // namespace eva
