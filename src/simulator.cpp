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

bool Simulator::handleCommand(const std::string& line) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;

    if (command == "quit" || command == "exit") {
        return false;
    }
    if (command == "reset") {
        o2_ = kNominalO2;
        pressure_ = kNominalPressure;
        thermal_ = kNominalThermal;
        std::cout << "Reset all metrics to nominal.\n";
        return true;
    }

    double value = 0.0;
    if (!(iss >> value)) {
        if (!command.empty()) {
            std::cout << "Unrecognized command: " << line << "\n";
        }
        return true;
    }

    if (command == "o2") {
        o2_ = value;
    } else if (command == "pressure") {
        pressure_ = value;
    } else if (command == "thermal") {
        thermal_ = value;
    } else {
        std::cout << "Unrecognized command: " << line << "\n";
    }

    return true;
}

void Simulator::publishLoop() {
    while (running_) {
        publishFrame();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Simulator::publishFrame() {
    SuitTelemetryFrame frame{suitId_, o2_, pressure_, thermal_, std::chrono::system_clock::now()};
    bus_.publish("telemetry.suit", frame);
}

} // namespace eva
