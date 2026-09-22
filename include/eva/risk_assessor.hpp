#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

// Correlates alerts across O2/Pressure/Thermal per suit. Individual monitors
// only reason about their own metric; this composes their independently
// published signals into a higher-level diagnosis (multiple simultaneous
// anomalies) that none of them could raise alone, demonstrating why the
// pub/sub bus matters architecturally, not just as a delivery mechanism.
class RiskAssessor {
public:
    explicit RiskAssessor(MessageBus& bus);

private:
    struct SuitState {
        std::unordered_map<std::string, AlertSeverity> metricLevels;
        SuitTelemetryFrame lastFrame{};
        AlertSeverity lastComposite = AlertSeverity::Info;
    };

    void onTelemetry(const std::any& payload);
    void onAlert(const std::any& payload);
    void evaluate(const std::string& suitId);

    MessageBus& bus_;
    std::unordered_map<std::string, SuitState> suits_;
};

} // namespace eva
