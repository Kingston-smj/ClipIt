cmake_minimum_required(VERSION 3.20)
project(ClipIt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_standard_project_setup()

# ── Source files (always compiled) ───────────────────────────────────────────
set(CLIPIT_SOURCES
    src/main.cpp
    src/app/application_controller.cpp
    src/app/logging.cpp
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

# ── Optional X11 backend ──────────────────────────────────────────────────────
# Enabled automatically when libx11 is found; can be forced off with
#   cmake -DCLIPIT_ENABLE_X11=OFF ...
option(CLIPIT_ENABLE_X11 "Build the XGrabKey hotkey backend" ON)

if(CLIPIT_ENABLE_X11)
    find_package(X11 QUIET)
    if(X11_FOUND)
        message(STATUS "ClipIt: X11 found — building XGrabKey backend")
        list(APPEND CLIPIT_SOURCES src/platform/global_hotkey_x11.cpp)
        set(CLIPIT_X11_LIBS ${X11_LIBRARIES})
        set(CLIPIT_X11_INCLUDE_DIRS ${X11_INCLUDE_DIR})
        set(CLIPIT_X11_DEFINE CLIPIT_X11_BACKEND)
    else()
        message(STATUS "ClipIt: X11 not found — XGrabKey backend disabled")
    endif()
endif()

# ── Target ────────────────────────────────────────────────────────────────────
add_executable(clipit ${CLIPIT_SOURCES})

target_include_directories(clipit PRIVATE
    src
    ${CLIPIT_X11_INCLUDE_DIRS}
)

target_compile_definitions(clipit PRIVATE
    ${CLIPIT_X11_DEFINE}
)

target_link_libraries(clipit PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ${CLIPIT_X11_LIBS}
)

enable_testing()
add_subdirectory(tests)
