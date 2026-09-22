#include "eva/log_format.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace eva {

std::string currentTimestamp() {
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto nowTimeT = system_clock::to_time_t(now);
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &nowTimeT);
#else
    localtime_r(&nowTimeT, &localTm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTm, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string severityToString(AlertSeverity severity) {
    switch (severity) {
        case AlertSeverity::Info:
            return "INFO";
        case AlertSeverity::Warning:
            return "WARNING";
        case AlertSeverity::Critical:
            return "CRITICAL";
    }
    return "UNKNOWN";
}

std::string formatAlert(const std::string& label, const Alert& alert) {
    std::ostringstream oss;
    oss << "[" << currentTimestamp() << "] [" << label << ", " << severityToString(alert.severity) << "] "
        << alert.source << " (" << alert.suitId << "): " << alert.message << " -> "
        << alert.recommendedAction;
    return oss.str();
}

} // namespace eva
