find_package(Qt6 REQUIRED COMPONENTS Core Gui Test)

add_executable(test_history
    test_history.cpp
    ../src/core/clipboard_item.cpp
    ../src/core/clipboard_history.cpp
)

target_include_directories(test_history PRIVATE ../src)
target_link_libraries(test_history PRIVATE Qt6::Core Qt6::Gui Qt6::Test)

add_test(NAME test_history COMMAND test_history)

add_executable(test_shell_probe test_shell_probe.cpp)
target_include_directories(test_shell_probe PRIVATE ../src)
target_link_libraries(test_shell_probe PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_shell_probe COMMAND test_shell_probe)

add_executable(test_picker_categories
    test_picker_categories.cpp
    ../src/ui/picker_panel.cpp
    ../src/ui/emoji_panel.cpp
    ../src/ui/kaomoji_panel.cpp
    ../src/ui/symbol_panel.cpp
    ../src/app/logging.cpp
)
target_include_directories(test_picker_categories PRIVATE ../src)
target_link_libraries(test_picker_categories PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
add_test(NAME test_picker_categories COMMAND test_picker_categories)
