#pragma once

#include <chrono>
#include <string>

namespace eva {

enum class TelemetryType { O2, Pressure, Thermal };

// Placeholder for what would be a decoded SLE-RCF frame in the real system.
struct TelemetryFrame {
    TelemetryType type;
    std::string suitId;
    double value; // unit depends on type (see monitor thresholds)
    std::chrono::system_clock::time_point timestamp;
};

} // namespace eva
