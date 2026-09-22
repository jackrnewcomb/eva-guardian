#include "test_harness.hpp"

#include "eva/alert.hpp"
#include "eva/components/o2_monitor.hpp"
#include "eva/components/pressure_monitor.hpp"
#include "eva/message_bus.hpp"
#include "eva/risk_assessor.hpp"
#include "eva/telemetry_frame.hpp"
#include "eva/thresholds.hpp"

#include <any>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

// Collects alerts published to "alerts", polling briefly since the bus
// dispatches asynchronously on its own worker thread.
class AlertCollector {
public:
    explicit AlertCollector(eva::MessageBus& bus) {
        bus.subscribe("alerts", [this](const std::any& payload) {
            std::lock_guard<std::mutex> lock(mutex_);
            alerts_.push_back(std::any_cast<eva::Alert>(payload));
        });
    }

    std::vector<eva::Alert> waitForAtLeast(size_t count) {
        for (int i = 0; i < 200; ++i) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (alerts_.size() >= count) {
                    return alerts_;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::lock_guard<std::mutex> lock(mutex_);
        return alerts_;
    }

private:
    std::mutex mutex_;
    std::vector<eva::Alert> alerts_;
};

eva::SuitTelemetryFrame makeFrame(const std::string& suitId, double o2, double pressure,
                                   double thermal) {
    return eva::SuitTelemetryFrame{suitId, o2, pressure, thermal, std::chrono::system_clock::now()};
}

} // namespace

TEST_CASE(O2MonitorStaysSilentWhenNominal) {
    eva::MessageBus bus;
    eva::O2Monitor monitor(bus);
    AlertCollector collector(bus);

    bus.publish("telemetry.suit", makeFrame("suit-1", 20.9, 32.0, 24.0));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    CHECK(collector.waitForAtLeast(1).empty());
}

TEST_CASE(O2MonitorFiresAtCriticalBoundary) {
    eva::MessageBus bus;
    eva::O2Monitor monitor(bus);
    AlertCollector collector(bus);

    bus.publish("telemetry.suit", makeFrame("suit-1", eva::kO2CriticalPercent - 0.1, 32.0, 24.0));

    auto alerts = collector.waitForAtLeast(1);
    CHECK(alerts.size() == 1);
    if (!alerts.empty()) {
        CHECK(alerts[0].severity == eva::AlertSeverity::Critical);
        CHECK(alerts[0].metric == "O2");
    }
}

TEST_CASE(O2MonitorPerSuitStateDoesNotCrossContaminate) {
    eva::MessageBus bus;
    eva::O2Monitor monitor(bus);
    AlertCollector collector(bus);

    // suit-1 goes critical...
    bus.publish("telemetry.suit", makeFrame("suit-1", 17.0, 32.0, 24.0));
    // ...suit-2 stays nominal and should not produce its own alert.
    bus.publish("telemetry.suit", makeFrame("suit-2", 20.9, 32.0, 24.0));

    auto alerts = collector.waitForAtLeast(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CHECK(alerts.size() == 1);
    if (!alerts.empty()) {
        CHECK(alerts[0].suitId == "suit-1");
    }
}

TEST_CASE(RiskAssessorEscalatesOnlyWithTwoAbnormalMetrics) {
    eva::MessageBus bus;
    eva::O2Monitor o2Monitor(bus);
    eva::PressureMonitor pressureMonitor(bus);
    eva::RiskAssessor riskAssessor(bus);
    AlertCollector collector(bus);

    // Only O2 abnormal: no composite alert expected yet.
    bus.publish("telemetry.suit", makeFrame("suit-1", 17.0, 32.0, 24.0));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    bool sawCompositeAfterFirstMetric = false;
    for (const auto& alert : collector.waitForAtLeast(1)) {
        if (alert.source == "RiskAssessor") {
            sawCompositeAfterFirstMetric = true;
        }
    }
    CHECK(!sawCompositeAfterFirstMetric);

    // Now pressure also abnormal: composite should escalate.
    bus.publish("telemetry.suit", makeFrame("suit-1", 17.0, 25.0, 24.0));

    auto alerts = collector.waitForAtLeast(3); // O2 alert, Pressure alert, Composite alert
    bool sawComposite = false;
    for (const auto& alert : alerts) {
        if (alert.source == "RiskAssessor" && alert.severity == eva::AlertSeverity::Critical) {
            sawComposite = true;
        }
    }
    CHECK(sawComposite);
}
