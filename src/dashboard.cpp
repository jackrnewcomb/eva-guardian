#include "eva/dashboard.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace eva {

namespace {

const char* toString(AlertSeverity severity) {
    switch (severity) {
        case AlertSeverity::Info:
            return "OK";
        case AlertSeverity::Warning:
            return "WARN";
        case AlertSeverity::Critical:
            return "CRIT";
    }
    return "?";
}

} // namespace

Dashboard::Dashboard(MessageBus& bus) {
    bus.subscribe("telemetry.suit", [this](const std::any& payload) { onTelemetry(payload); });
    bus.subscribe("alerts", [this](const std::any& payload) { onAlert(payload); });
}

void Dashboard::onTelemetry(const std::any& payload) {
    const auto& frame = std::any_cast<const SuitTelemetryFrame&>(payload);
    std::lock_guard<std::mutex> lock(mutex_);
    suits_[frame.suitId].frame = frame;
}

void Dashboard::onAlert(const std::any& payload) {
    const auto& alert = std::any_cast<const Alert&>(payload);
    std::lock_guard<std::mutex> lock(mutex_);
    SuitStatus& status = suits_[alert.suitId];
    if (alert.metric == "O2") {
        status.o2Level = alert.severity;
    } else if (alert.metric == "Pressure") {
        status.pressureLevel = alert.severity;
    } else if (alert.metric == "Thermal") {
        status.thermalLevel = alert.severity;
    }
}

void Dashboard::render() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::cout << "\n--- EVA Guardian Dashboard ---\n";
    std::cout << std::left << std::setw(10) << "Suit" << std::setw(20) << "O2 (%)" << std::setw(22)
               << "Pressure (kPa)" << std::setw(20) << "Thermal (C)" << "\n";

    for (const auto& [suitId, status] : suits_) {
        std::ostringstream o2, pressure, thermal;
        o2 << status.frame.o2Percent << " [" << toString(status.o2Level) << "]";
        pressure << status.frame.pressureKpa << " [" << toString(status.pressureLevel) << "]";
        thermal << status.frame.thermalCelsius << " [" << toString(status.thermalLevel) << "]";

        std::cout << std::left << std::setw(10) << suitId << std::setw(20) << o2.str() << std::setw(22)
                   << pressure.str() << std::setw(20) << thermal.str() << "\n";
    }

    std::cout << "------------------------------\n";
}

} // namespace eva
