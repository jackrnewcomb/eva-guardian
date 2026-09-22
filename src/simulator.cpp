#include "eva/simulator.hpp"

#include <chrono>
#include <iostream>
#include <sstream>

namespace eva {

Simulator::Simulator(MessageBus& bus, std::string suitId)
    : bus_(bus), suitId_(std::move(suitId)), o2_(kNominalO2), pressure_(kNominalPressure),
      thermal_(kNominalThermal) {}

Simulator::~Simulator() { stop(); }

void Simulator::start() {
    if (running_.exchange(true)) {
        return;
    }
    worker_ = std::thread(&Simulator::publishLoop, this);
}

void Simulator::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void Simulator::handleCommand(const std::string& line) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;

    if (command == "reset") {
        o2_ = kNominalO2;
        pressure_ = kNominalPressure;
        thermal_ = kNominalThermal;
        o2Rate_ = 0.0;
        pressureRate_ = 0.0;
        thermalRate_ = 0.0;
        std::cout << "[" << suitId_ << "] Reset all metrics to nominal.\n";
        return;
    }

    if (command != "o2" && command != "pressure" && command != "thermal") {
        if (!command.empty()) {
            std::cout << "Unrecognized command: " << line << "\n";
        }
        return;
    }

    std::string next;
    iss >> next;

    if (next == "drift") {
        double rate = 0.0;
        if (!(iss >> rate)) {
            std::cout << "Usage: " << command << " drift <rate-per-second>\n";
            return;
        }
        if (command == "o2") {
            o2Rate_ = rate;
        } else if (command == "pressure") {
            pressureRate_ = rate;
        } else {
            thermalRate_ = rate;
        }
        std::cout << "[" << suitId_ << "] " << command << " drifting at " << rate << "/s\n";
        return;
    }

    double value = 0.0;
    try {
        value = std::stod(next);
    } catch (const std::exception&) {
        std::cout << "Unrecognized command: " << line << "\n";
        return;
    }

    if (command == "o2") {
        o2_ = value;
        o2Rate_ = 0.0;
    } else if (command == "pressure") {
        pressure_ = value;
        pressureRate_ = 0.0;
    } else {
        thermal_ = value;
        thermalRate_ = 0.0;
    }
}

void Simulator::publishLoop() {
    while (running_) {
        o2_ = o2_.load() + o2Rate_.load();
        pressure_ = pressure_.load() + pressureRate_.load();
        thermal_ = thermal_.load() + thermalRate_.load();

        publishFrame();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Simulator::publishFrame() {
    SuitTelemetryFrame frame{suitId_, o2_, pressure_, thermal_, std::chrono::system_clock::now()};
    bus_.publish("telemetry.suit", frame);
}

} // namespace eva
