#include "eva/trend_predictor.hpp"

#include <sstream>

#include "eva/alert.hpp"
#include "eva/thresholds.hpp"

namespace eva {

TrendPredictor::TrendPredictor(MessageBus& bus) : bus_(bus) {
    bus_.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
}

void TrendPredictor::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);
    const auto now = std::chrono::steady_clock::now();

    auto it = lastSampleBySuit_.find(frame.suitId);
    if (it == lastSampleBySuit_.end()) {
        lastSampleBySuit_[frame.suitId] = Sample{frame, now};
        return;
    }

    const Sample previous = it->second;
    it->second = Sample{frame, now};

    const double dtSeconds = std::chrono::duration<double>(now - previous.observedAt).count();
    if (dtSeconds <= 0.0) {
        return;
    }

    checkMetric(frame.suitId, "O2", previous.frame.o2Percent, frame.o2Percent, dtSeconds, true,
                kO2CriticalPercent, false, 0.0, frame);
    checkMetric(frame.suitId, "Pressure", previous.frame.pressureKpa, frame.pressureKpa, dtSeconds, true,
                kPressureCriticalMinKpa, true, kPressureCriticalMaxKpa, frame);
    checkMetric(frame.suitId, "Thermal", previous.frame.thermalCelsius, frame.thermalCelsius, dtSeconds,
                true, kThermalCriticalMinCelsius, true, kThermalCriticalMaxCelsius, frame);
}

void TrendPredictor::checkMetric(const std::string& suitId, const std::string& metric, double previous,
                                    double current, double dtSeconds, bool hasLowCritical,
                                    double lowCritical, bool hasHighCritical, double highCritical,
                                    const SuitTelemetryFrame& frame) {
    const double rate = (current - previous) / dtSeconds;

    double timeToCritical = -1.0;
    if (hasLowCritical && rate < 0.0 && current > lowCritical) {
        timeToCritical = (current - lowCritical) / (-rate);
    } else if (hasHighCritical && rate > 0.0 && current < highCritical) {
        timeToCritical = (highCritical - current) / rate;
    }

    const std::string key = suitId + "|" + metric;
    const bool trendingToCritical = timeToCritical >= 0.0 && timeToCritical <= kHorizonSeconds;

    if (!trendingToCritical) {
        warnedBySuitMetric_[key] = false;
        return;
    }

    if (warnedBySuitMetric_[key]) {
        return;
    }
    warnedBySuitMetric_[key] = true;

    std::ostringstream message;
    message << "Projected to reach critical " << metric << " in ~" << static_cast<int>(timeToCritical)
             << "s at current rate";

    bus_.publish("alerts", Alert{"TrendPredictor", suitId, metric, AlertSeverity::Warning, message.str(),
                                  "Prepare corrective action now; do not wait for the threshold to be crossed.",
                                  frame});
}

} // namespace eva
