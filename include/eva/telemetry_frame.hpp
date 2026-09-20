#pragma once

#include <chrono>
#include <string>

namespace eva {

// Placeholder for what would be a decoded SLE-RCF frame in the real system.
// A real suit reports all of its housekeeping metrics together per sample
// interval, so we model one frame per suit per tick rather than one frame
// per metric.
struct SuitTelemetryFrame {
    std::string suitId;
    double o2Percent;
    double pressureKpa;
    double thermalCelsius;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace eva
