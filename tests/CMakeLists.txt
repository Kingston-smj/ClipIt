find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_history
    test_history.cpp
    ../src/core/clipboard_history.cpp
)

target_include_directories(test_history PRIVATE ../src)
target_link_libraries(test_history PRIVATE Qt6::Core Qt6::Test)

add_test(NAME test_history COMMAND test_history)
