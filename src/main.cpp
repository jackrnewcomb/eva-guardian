#include <any>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "eva/alert.hpp"
#include "eva/components/o2_monitor.hpp"
#include "eva/components/pressure_monitor.hpp"
#include "eva/components/thermal_monitor.hpp"
#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace {

const char* toString(eva::AlertSeverity severity) {
    switch (severity) {
        case eva::AlertSeverity::Warning:
            return "WARNING";
        case eva::AlertSeverity::Critical:
            return "CRITICAL";
    }
    return "UNKNOWN";
}

// TODO(visual studio): replace with a real/interactive simulator (e.g. CLI
// driven) that can inject live anomalies during a demo.
std::vector<eva::TelemetryFrame> makeMockFrames() {
    const auto now = std::chrono::system_clock::now();
    return {
        {eva::TelemetryType::O2, "suit-1", 20.9, now},
        {eva::TelemetryType::Pressure, "suit-1", 32.0, now},
        {eva::TelemetryType::Thermal, "suit-1", 24.0, now},
        {eva::TelemetryType::O2, "suit-1", 17.8, now}, // triggers alert
        {eva::TelemetryType::Pressure, "suit-1", 25.0, now}, // triggers alert
        {eva::TelemetryType::Thermal, "suit-1", 40.0, now}, // triggers alert
    };
}

const char* topicFor(eva::TelemetryType type) {
    switch (type) {
        case eva::TelemetryType::O2:
            return "telemetry.o2";
        case eva::TelemetryType::Pressure:
            return "telemetry.pressure";
        case eva::TelemetryType::Thermal:
            return "telemetry.thermal";
    }
    return "telemetry.unknown";
}

} // namespace

int main() {
    eva::MessageBus bus;

    eva::O2Monitor o2Monitor(bus);
    eva::PressureMonitor pressureMonitor(bus);
    eva::ThermalMonitor thermalMonitor(bus);

    bus.subscribe("alerts", [](const std::any& payload) {
        const auto& alert = std::any_cast<const eva::Alert&>(payload);
        std::cout << "[" << toString(alert.severity) << "] " << alert.source << " (" << alert.suitId
                   << "): " << alert.message << " (value=" << alert.frame.value << ")\n";
    });

    for (const auto& frame : makeMockFrames()) {
        bus.publish(topicFor(frame.type), frame);
    }

    // Give the async dispatch thread time to process the mock frames before exiting.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;
}
