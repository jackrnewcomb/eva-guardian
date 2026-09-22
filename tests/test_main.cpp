#include "test_harness.hpp"

int main() {
    for (const TestCase& test : testRegistry()) {
        std::cout << "Running " << test.name << "...\n";
        test.fn();
    }

    if (failureCount() > 0) {
        std::cerr << failureCount() << " check(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
