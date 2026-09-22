add_executable(eva_tests
    test_main.cpp
    test_message_bus.cpp
    test_components.cpp
)

target_link_libraries(eva_tests PRIVATE eva_core)

add_test(NAME eva_tests COMMAND eva_tests)
