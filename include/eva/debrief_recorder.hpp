#pragma once

#include <any>
#include <array>
#include <chrono>
#include <limits>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

// Tracks per-suit min/max telemetry, time spent in each severity state per
// metric, and alert tallies, then prints a post-EVA debrief summary on
// demand. Purely a reporting subscriber on the same bus; it doesn't affect
// any other component's logic.
class DebriefRecorder {
public:
    explicit DebriefRecorder(MessageBus& bus);

    void printReport();

private:
    struct MetricStats {
        double minValue = std::numeric_limits<double>::infinity();
        double maxValue = -std::numeric_limits<double>::infinity();
        AlertSeverity currentSeverity = AlertSeverity::Info;
        std::chrono::steady_clock::time_point lastChangeTime{};
        std::array<double, 3> secondsInSeverity{0.0, 0.0, 0.0}; // indexed by AlertSeverity
        int alertCount = 0;
    };

    struct SuitStats {
        std::unordered_map<std::string, MetricStats> metrics; // "O2"/"Pressure"/"Thermal"
        int predictiveWarnings = 0;
        int compositeEscalations = 0;
    };

    void onTelemetry(const std::any& payload);
    void onAlert(const std::any& payload);
    void touchMetric(SuitStats& suit, const std::string& metric, double value);

    std::unordered_map<std::string, SuitStats> suits_;
};

} // namespace eva
