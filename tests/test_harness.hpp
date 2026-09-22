#pragma once

// Minimal self-registering test harness so tests don't need an external
// framework dependency. Each TEST_CASE registers itself; test_main.cpp runs
// them all and reports pass/fail.

#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& testRegistry() {
    static std::vector<TestCase> registry;
    return registry;
}

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> fn) {
        testRegistry().push_back({name, std::move(fn)});
    }
};

inline int& failureCount() {
    static int count = 0;
    return count;
}

#define TEST_CASE(name)                                            \
    void name();                                                   \
    static TestRegistrar registrar_##name(#name, name); /* NOLINT */ \
    void name()

#define CHECK(cond)                                                                       \
    do {                                                                                   \
        if (!(cond)) {                                                                     \
            std::cerr << "CHECK failed: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n"; \
            ++failureCount();                                                              \
        }                                                                                   \
    } while (0)
