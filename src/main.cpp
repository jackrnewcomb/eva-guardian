#include <any>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/alert_logger.hpp"
#include "eva/components/o2_monitor.hpp"
#include "eva/components/pressure_monitor.hpp"
#include "eva/components/thermal_monitor.hpp"
#include "eva/dashboard.hpp"
#include "eva/debrief_recorder.hpp"
#include "eva/earth_relay.hpp"
#include "eva/log_format.hpp"
#include "eva/message_bus.hpp"
#include "eva/risk_assessor.hpp"
#include "eva/simulator.hpp"
#include "eva/trend_predictor.hpp"

int main() {
    // Force line-by-line flushing so output appears immediately even when
    // stdout is redirected/piped (e.g. by a scripted demo), not just when
    // attached to a real console.
    std::cout.setf(std::ios_base::unitbuf);

    eva::MessageBus bus;

    eva::O2Monitor o2Monitor(bus);
    eva::PressureMonitor pressureMonitor(bus);
    eva::ThermalMonitor thermalMonitor(bus);
    eva::Dashboard dashboard(bus);
    eva::RiskAssessor riskAssessor(bus);
    eva::TrendPredictor trendPredictor(bus);
    eva::DebriefRecorder debriefRecorder(bus);
    eva::AlertLogger alertLogger(bus, "eva_guardian.log");

    // Stand-in for the naive "relay to Earth and wait" alternative this
    // product replaces, contrasted against the instant LOCAL alert below.
    eva::EarthRelay earthRelay(bus, std::chrono::seconds(8));

    bus.subscribe("alerts", [](const std::any& payload) {
        const auto& alert = std::any_cast<const eva::Alert&>(payload);
        std::cout << eva::formatAlert("LOCAL", alert) << "\n";
    });

    std::unordered_map<std::string, std::unique_ptr<eva::Simulator>> simulators;
    for (const std::string& suitId : {"suit-1", "suit-2", "suit-3"}) {
        auto simulator = std::make_unique<eva::Simulator>(bus, suitId);
        simulator->start();
        simulators.emplace(suitId, std::move(simulator));
    }

    std::cout << "EVA Guardian running with suits: suit-1, suit-2, suit-3\n";
    std::cout << "Commands: <suit> o2|pressure|thermal <value>, <suit> o2|pressure|thermal drift "
                 "<rate/s>, <suit> reset, dashboard, quit\n";
    std::cout << "Watch for [LOCAL] alerts (instant) vs [EARTH RELAY] alerts (~8s simulated delay)\n";
    std::cout << "'quit' prints a post-EVA debrief summary before exiting\n";

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

    debriefRecorder.printReport();

    return 0;
}
