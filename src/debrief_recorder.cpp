#include "eva/debrief_recorder.hpp"

#include <algorithm>
#include <iostream>

namespace eva {

DebriefRecorder::DebriefRecorder(MessageBus& bus) {
    bus.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
    bus.subscribe("alerts", [this](const std::any& payload) { onAlert(payload); });
}

void DebriefRecorder::touchMetric(SuitStats& suit, const std::string& metric, double value) {
    MetricStats& stats = suit.metrics[metric];
    if (stats.lastChangeTime.time_since_epoch().count() == 0) {
        stats.lastChangeTime = std::chrono::steady_clock::now();
    }
    stats.minValue = std::min(stats.minValue, value);
    stats.maxValue = std::max(stats.maxValue, value);
}

void DebriefRecorder::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);
    SuitStats& suit = suits_[frame.suitId];
    touchMetric(suit, "O2", frame.o2Percent);
    touchMetric(suit, "Pressure", frame.pressureKpa);
    touchMetric(suit, "Thermal", frame.thermalCelsius);
}

void DebriefRecorder::onAlert(const std::any& payload) {
    const auto& alert = std::any_cast<const Alert&>(payload);
    SuitStats& suit = suits_[alert.suitId];

    if (alert.source == "TrendPredictor") {
        suit.predictiveWarnings++;
        return;
    }
    if (alert.source == "RiskAssessor") {
        if (alert.severity == AlertSeverity::Critical) {
            suit.compositeEscalations++;
        }
        return;
    }

    MetricStats& stats = suit.metrics[alert.metric];
    const auto now = std::chrono::steady_clock::now();
    if (stats.lastChangeTime.time_since_epoch().count() != 0) {
        const double elapsed = std::chrono::duration<double>(now - stats.lastChangeTime).count();
        stats.secondsInSeverity[static_cast<int>(stats.currentSeverity)] += elapsed;
    }
    stats.currentSeverity = alert.severity;
    stats.lastChangeTime = now;
    stats.alertCount++;
}

void DebriefRecorder::printReport() {
    const auto now = std::chrono::steady_clock::now();

    std::cout << "\n=== Post-EVA Debrief ===\n";
    for (auto& [suitId, suit] : suits_) {
        std::cout << "Suit " << suitId << ":\n";
        for (const char* metric : {"O2", "Pressure", "Thermal"}) {
            auto it = suit.metrics.find(metric);
            if (it == suit.metrics.end()) {
                continue;
            }
            MetricStats& stats = it->second;

            // Finalize time spent in whatever severity was active at exit.
            if (stats.lastChangeTime.time_since_epoch().count() != 0) {
                const double elapsed = std::chrono::duration<double>(now - stats.lastChangeTime).count();
                stats.secondsInSeverity[static_cast<int>(stats.currentSeverity)] += elapsed;
                stats.lastChangeTime = now;
            }

            std::cout << "  " << metric << ": min=" << stats.minValue << " max=" << stats.maxValue
                       << " time[Info=" << stats.secondsInSeverity[0] << "s Warning="
                       << stats.secondsInSeverity[1] << "s Critical=" << stats.secondsInSeverity[2]
                       << "s] alerts=" << stats.alertCount << "\n";
        }
        std::cout << "  Predictive warnings issued: " << suit.predictiveWarnings << "\n";
        std::cout << "  Composite (multi-system) escalations: " << suit.compositeEscalations << "\n";
    }
    std::cout << "========================\n";
}

} // namespace eva
