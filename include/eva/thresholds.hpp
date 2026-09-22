#pragma once

namespace eva {

// Shared safety thresholds so reactive monitors and predictive components
// can't drift out of sync with each other.

inline constexpr double kO2WarnPercent = 19.5;
inline constexpr double kO2CriticalPercent = 18.0;

inline constexpr double kPressureWarnMinKpa = 30.0;
inline constexpr double kPressureWarnMaxKpa = 34.0;
inline constexpr double kPressureCriticalMinKpa = 28.0;
inline constexpr double kPressureCriticalMaxKpa = 36.0;

inline constexpr double kThermalWarnMinCelsius = 18.0;
inline constexpr double kThermalWarnMaxCelsius = 32.0;
inline constexpr double kThermalCriticalMinCelsius = 15.0;
inline constexpr double kThermalCriticalMaxCelsius = 35.0;

} // namespace eva
