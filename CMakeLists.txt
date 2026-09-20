cmake_minimum_required(VERSION 3.16)
project(EvaGuardian CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(eva_guardian
    src/main.cpp
    src/message_bus.cpp
    src/components/o2_monitor.cpp
    src/components/pressure_monitor.cpp
    src/components/thermal_monitor.cpp
)

target_include_directories(eva_guardian PRIVATE include)
target_link_libraries(eva_guardian PRIVATE Threads::Threads)
