#pragma once

#include <string>

#include "eva/telemetry_frame.hpp"

namespace eva {

enum class AlertSeverity { Warning, Critical };

struct Alert {
    std::string source; // component that raised it, e.g. "O2Monitor"
    std::string suitId;
    AlertSeverity severity;
    std::string message;
    TelemetryFrame frame; // the frame that triggered it
};

} // namespace eva
