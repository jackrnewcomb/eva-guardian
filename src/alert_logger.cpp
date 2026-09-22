#include "eva/alert_logger.hpp"

#include "eva/alert.hpp"
#include "eva/log_format.hpp"

namespace eva {

AlertLogger::AlertLogger(MessageBus& bus, const std::string& path) : file_(path, std::ios::trunc) {
    bus.subscribe("alerts", [this](const std::any& payload) { onAlert(payload); });
}

void AlertLogger::onAlert(const std::any& payload) {
    const auto& alert = std::any_cast<const Alert&>(payload);
    file_ << formatAlert("LOG", alert) << '\n';
    file_.flush();
}

} // namespace eva
