#include <any>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/components/o2_monitor.hpp"
#include "eva/components/pressure_monitor.hpp"
#include "eva/components/thermal_monitor.hpp"
#include "eva/dashboard.hpp"
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
    eva::Dashboard dashboard(bus);

    bus.subscribe("alerts", [](const std::any& payload) {
        const auto& alert = std::any_cast<const eva::Alert&>(payload);
        std::cout << "[" << toString(alert.severity) << "] " << alert.source << " (" << alert.suitId
                   << "): " << alert.message << " -> " << alert.recommendedAction << "\n";
    });

    std::unordered_map<std::string, std::unique_ptr<eva::Simulator>> simulators;
    for (const std::string& suitId : {"suit-1", "suit-2", "suit-3"}) {
        auto simulator = std::make_unique<eva::Simulator>(bus, suitId);
        simulator->start();
        simulators.emplace(suitId, std::move(simulator));
    }

    std::cout << "EVA Guardian running with suits: suit-1, suit-2, suit-3\n";
    std::cout << "Commands: <suit> o2|pressure|thermal <value>, <suit> reset, dashboard, quit\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string first;
        iss >> first;

        if (first == "quit" || first == "exit") {
            break;
        }
        if (first == "dashboard") {
            dashboard.render();
            continue;
        }

        auto it = simulators.find(first);
        if (it == simulators.end()) {
            std::cout << "Unknown suit or command: " << first << ". Known suits:";
            for (const auto& [suitId, unused] : simulators) {
                std::cout << " " << suitId;
            }
            std::cout << "\n";
            continue;
        }

        std::string rest;
        std::getline(iss, rest);
        if (!rest.empty() && rest.front() == ' ') {
            rest.erase(0, 1);
        }
        it->second->handleCommand(rest);
    }

    for (auto& [suitId, simulator] : simulators) {
        simulator->stop();
    }

    return 0;
}
