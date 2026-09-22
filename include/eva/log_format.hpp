#pragma once

#include <string>

#include "eva/alert.hpp"

namespace eva {

// Human-readable "HH:MM:SS.mmm" timestamp for the current instant.
std::string currentTimestamp();

std::string severityToString(AlertSeverity severity);

// Consistent one-line format shared by every place an alert gets printed or
// logged: "[HH:MM:SS.mmm] [label, SEVERITY] source (suitId): message -> action"
std::string formatAlert(const std::string& label, const Alert& alert);

} // namespace eva
