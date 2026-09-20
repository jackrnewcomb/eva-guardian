#include "eva/components/o2_monitor.hpp"

#include "eva/alert.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

O2Monitor::O2Monitor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void O2Monitor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);

    if (frame.o2Percent < kMinSafeO2Percent) {
        bus_.publish("alerts", Alert{
            "O2Monitor",
            frame.suitId,
            AlertSeverity::Critical,
            "O2 level below safe threshold",
            frame,
        });
    }
}

} // namespace eva
