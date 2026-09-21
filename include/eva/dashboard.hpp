#pragma once

#include <any>
#include <mutex>
#include <string>
#include <unordered_map>

#include "eva/alert.hpp"
#include "eva/message_bus.hpp"
#include "eva/telemetry_frame.hpp"

namespace eva {

// Tracks latest telemetry + alert severity per suit and renders a console
// snapshot on demand. Stands in for a habitat dashboard: in a real
// deployment this would just be another subscriber on the same bus, not
// something the engine has to own.
class Dashboard {
public:
    explicit Dashboard(MessageBus& bus);

    void render() const;

private:
    struct SuitStatus {
        SuitTelemetryFrame frame{};
        AlertSeverity o2Level = AlertSeverity::Info;
        AlertSeverity pressureLevel = AlertSeverity::Info;
        AlertSeverity thermalLevel = AlertSeverity::Info;
    };

    void onTelemetry(const std::any& payload);
    void onAlert(const std::any& payload);

    mutable std::mutex mutex_;
    std::unordered_map<std::string, SuitStatus> suits_;
};

} // namespace eva
