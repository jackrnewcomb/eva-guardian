#pragma once

#include <any>
#include <chrono>
#include <string>
#include <unordered_map>

#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

// Projects each metric's rate of change forward to estimate time-to-critical,
// so the crew gets a heads-up before a threshold is actually crossed instead
// of only after (the reactive monitors already cover the "already crossed"
// case).
class TrendPredictor {
public:
    explicit TrendPredictor(MessageBus& bus);

private:
    struct Sample {
        SuitTelemetryFrame frame;
        std::chrono::steady_clock::time_point observedAt;
    };

    void onTelemetry(const std::any& payload);
    void checkMetric(const std::string& suitId, const std::string& metric, double previous,
                      double current, double dtSeconds, bool hasLowCritical, double lowCritical,
                      bool hasHighCritical, double highCritical, const SuitTelemetryFrame& frame);

    MessageBus& bus_;
    std::unordered_map<std::string, Sample> lastSampleBySuit_;
    std::unordered_map<std::string, bool> warnedBySuitMetric_;

    static constexpr double kHorizonSeconds = 60.0;
};

} // namespace eva
