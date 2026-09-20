#include <any>
#include <iostream>
#include <string>

#include "eva/alert.hpp"
#include "eva/components/o2_monitor.hpp"
#include "eva/components/pressure_monitor.hpp"
#include "eva/components/thermal_monitor.hpp"
#include "eva/message_bus.hpp"
#include "eva/simulator.hpp"

namespace {

const char* toString(eva::AlertSeverity severity) {
    switch (severity) {
        case eva::AlertSeverity::Info:
            return "INFO";
        case eva::AlertSeverity::Warning:
            return "WARNING";
        case eva::AlertSeverity::Critical:
            return "CRITICAL";
    }
    return "UNKNOWN";
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
                   << "): " << alert.message << " (o2=" << alert.frame.o2Percent
                   << ", pressure=" << alert.frame.pressureKpa << ", thermal=" << alert.frame.thermalCelsius
                   << ")\n";
    });

    eva::Simulator simulator(bus, "suit-1");
    simulator.start();

    std::cout << "EVA Guardian running. Commands: o2 <value>, pressure <value>, thermal <value>, "
                 "reset, quit\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!simulator.handleCommand(line)) {
            break;
        }
    }

    simulator.stop();
    return 0;
}
