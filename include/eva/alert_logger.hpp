#pragma once

#include <any>
#include <fstream>
#include <string>

#include "eva/message_bus.hpp"

namespace eva {

// Writes every alert to a log file, formatted the same way as console
// output, so a demo session leaves a durable artifact behind.
class AlertLogger {
public:
    AlertLogger(MessageBus& bus, const std::string& path);

private:
    void onAlert(const std::any& payload);

    std::ofstream file_;
};

} // namespace eva
