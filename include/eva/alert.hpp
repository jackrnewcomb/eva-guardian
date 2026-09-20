#pragma once

#include <string>

#include "eva/telemetry_frame.hpp"

namespace eva {

// Info marks a recovery ("back to nominal") notice; Warning/Critical mark anomalies.
enum class AlertSeverity { Info, Warning, Critical };

struct Alert {
    std::string source; // component that raised it, e.g. "O2Monitor"
    std::string suitId;
    AlertSeverity severity;
    std::string message;
    SuitTelemetryFrame frame; // the frame that triggered it
};

} // namespace eva
