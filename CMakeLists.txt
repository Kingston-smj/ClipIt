cmake_minimum_required(VERSION 3.20)
project(ClipIt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_executable(clipit
    src/main.cpp
    src/app/application_controller.cpp
    src/core/clipboard_history.cpp
    src/platform/clipboard_watcher_qt.cpp
    src/platform/global_hotkey_stub.cpp
    src/ui/history_model.cpp
    src/ui/history_popup.cpp
)

target_include_directories(clipit PRIVATE src)
target_link_libraries(clipit PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
