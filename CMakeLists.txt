cmake_minimum_required(VERSION 3.20)
project(ClipIt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network DBus)

qt_standard_project_setup()

# ── Source files (always compiled) ───────────────────────────────────────────
set(CLIPIT_SOURCES
    src/main.cpp
    src/app/application_controller.cpp
    src/app/logging.cpp
    src/platform/gnome_shell_bridge.cpp
    src/core/clipboard_item.cpp
    src/core/clipboard_history.cpp
    src/platform/clipboard_watcher_qt.cpp
    src/platform/global_hotkey.cpp        # MOC compilation unit for base class
    src/platform/global_hotkey_stub.cpp   # contains GlobalHotkey::create()
    src/platform/global_hotkey_socket.cpp
    src/ui/history_model.cpp
    src/ui/history_popup.cpp
    src/ui/picker_panel.cpp
    src/ui/emoji_panel.cpp
    src/ui/kaomoji_panel.cpp
    src/ui/symbol_panel.cpp
)

# ── Target ────────────────────────────────────────────────────────────────────
add_executable(clipit ${CLIPIT_SOURCES})

qt_add_resources(clipit "clipit-icon"
    PREFIX "/"
    FILES assets/clipit-icon.png
)

target_include_directories(clipit PRIVATE
    src
)

target_link_libraries(clipit PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::DBus
)

enable_testing()
add_subdirectory(tests)
