cmake_minimum_required(VERSION 3.16)
project(EvaGuardian CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(eva_core STATIC
    src/message_bus.cpp
    src/simulator.cpp
    src/dashboard.cpp
    src/earth_relay.cpp
    src/risk_assessor.cpp
    src/trend_predictor.cpp
    src/debrief_recorder.cpp
    src/log_format.cpp
    src/alert_logger.cpp
    src/components/o2_monitor.cpp
    src/components/pressure_monitor.cpp
    src/components/thermal_monitor.cpp
)

target_include_directories(eva_core PUBLIC include)
target_link_libraries(eva_core PUBLIC Threads::Threads)

add_executable(eva_guardian src/main.cpp)
target_link_libraries(eva_guardian PRIVATE eva_core)

enable_testing()
add_subdirectory(tests)
