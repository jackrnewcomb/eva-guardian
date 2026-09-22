#include "eva/risk_assessor.hpp"

#include <sstream>

namespace eva {

RiskAssessor::RiskAssessor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
    bus_.subscribe("alerts", [this](const std::any& payload) { onAlert(payload); });
}

void RiskAssessor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);
    suits_[frame.suitId].lastFrame = frame;
}

void RiskAssessor::onAlert(const std::any& payload) {
    const auto& alert = std::any_cast<const Alert&>(payload);

    // Only correlate the three primary monitors; ignore our own composite alerts.
    if (alert.metric != "O2" && alert.metric != "Pressure" && alert.metric != "Thermal") {
        return;
    }

    suits_[alert.suitId].metricLevels[alert.metric] = alert.severity;
    evaluate(alert.suitId);
}

void RiskAssessor::evaluate(const std::string& suitId) {
    SuitState& state = suits_[suitId];

    int abnormalCount = 0;
    std::ostringstream abnormalMetrics;
    for (const auto& [metric, level] : state.metricLevels) {
        if (level != AlertSeverity::Info) {
            if (abnormalCount > 0) {
                abnormalMetrics << ", ";
            }
            abnormalMetrics << metric;
            ++abnormalCount;
        }
    }

    const AlertSeverity level = abnormalCount >= 2 ? AlertSeverity::Critical : AlertSeverity::Info;
    if (level == state.lastComposite) {
        return;
    }
    state.lastComposite = level;

    std::string message;
    std::string action;
    if (level == AlertSeverity::Critical) {
        message = "Multiple simultaneous anomalies detected (" + abnormalMetrics.str() + ")";
        action = "Recommend full EVA abort and immediate return to habitat, not just individual "
                 "system correction.";
    } else {
        message = "All monitored systems back to nominal";
        action = "No action needed.";
    }

    bus_.publish("alerts",
                 Alert{"RiskAssessor", suitId, "Composite", level, message, action, state.lastFrame});
}

} // namespace eva
