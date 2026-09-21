#pragma once

#include <string>

#include "eva/telemetry_frame.hpp"

namespace eva {

// Info marks a recovery ("back to nominal") notice; Warning/Critical mark anomalies.
enum class AlertSeverity { Info, Warning, Critical };

struct Alert {
    std::string source; // component that raised it, e.g. "O2Monitor"
    std::string suitId;
    std::string metric; // "O2", "Pressure", or "Thermal"
    AlertSeverity severity;
    std::string message;
    std::string recommendedAction; // decision-support guidance for the crew
    SuitTelemetryFrame frame; // the frame that triggered it
};

} // namespace eva
