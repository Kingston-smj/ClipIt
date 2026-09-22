# ClipIt

A lightweight, cross-platform clipboard manager built with Qt/C++ that replicates the Windows **Win + V** clipboard history feature, with a strong focus on **memory efficiency** and **minimal background overhead**.

---

## Overview

ClipIt monitors your system clipboard, stores a bounded history of copied items, and allows quick access via a popup interface.

The project is designed as a **systems-level learning exercise** with production-oriented architecture:

* Clean separation of concerns (Core / Platform / UI)
* Event-driven (no polling)
* Strict memory constraints
* Cross-platform extensibility

---

## Current Features (MVP)

* Clipboard monitoring (text support)
* Fixed-size history (default: 10 items)
* Duplicate prevention
* Popup UI to view history
* Select item to restore to clipboard
* Qt Model/View architecture

---

## Planned Features

* Global shortcut (**Super + V**) — see strategy below
* Support for:

  * HTML content
  * File paths (URI list)
  * Images (compressed storage)
* Search/filter inside history
* Emoji, kaomoji, and symbol pickers (no-network, compile-time data)
* Persistent storage (optional, bounded)
* Tray integration + autostart
* Cross-platform support (Windows, macOS)

---

## Global Shortcut Strategy

ClipIt does **not** attempt compositor-specific key grabbing. Instead it exposes two
complementary trigger mechanisms:

### 1. Socket / D-Bus trigger *(primary — works on any Wayland compositor)*

ClipIt listens on a Unix domain socket at:

```
$XDG_RUNTIME_DIR/clipit.sock
```

Any message arriving on the socket triggers the popup. You bind **Super + V** once
in your DE's own shortcut settings, pointing at a tiny launcher command:

| Environment | How to bind |
|---|---|
| **GNOME** | Settings → Keyboard → Custom Shortcuts → command below |
| **KDE Plasma** | System Settings → Shortcuts → Custom Shortcuts → command below |
| **Hyprland** | `bind = SUPER, V, exec, <command>` in `hyprland.conf` |
| **sxhkd / xbindkeys** | Add keybind → command in `sxhkdrc` / `.xbindkeysrc` |

Trigger command (copy this into your shortcut):

```bash
echo '' | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/clipit.sock
```

This approach works on **GNOME Wayland, KDE Wayland, Hyprland, Sway, X11** — any
setup that can launch a shell command from a key press.

### 2. Raw `XGrabKey` *(secondary — X11 / XWayland only)*

When ClipIt detects it is running under X11 (`XDG_SESSION_TYPE=x11`), it
additionally registers **Super + V** directly via `XGrabKey`. This means the
shortcut works out of the box on pure X11 desktops without any manual binding.

On Wayland sessions this path is skipped entirely — use the socket trigger above.

---

## Architecture

### 1. Core (Platform-Agnostic)

Handles all memory-sensitive logic.

* `ClipboardHistory` – bounded ring buffer
* `ClipboardItem` – compact data representation
* `MemoryPolicy` – limits for items and bytes

Responsibilities:

* Store clipboard entries
* Enforce memory constraints
* Deduplicate entries

---

### 2. Platform Layer

Handles OS-specific interactions.

* `ClipboardWatcherQt` – listens for clipboard changes
* `GlobalHotkeySocket` – Unix socket listener (primary, Wayland-compatible)
* `GlobalHotkeyX11` – `XGrabKey` listener (secondary, X11 / XWayland only)
* `GlobalHotkey::create()` – factory that selects the right backend at runtime

Responsibilities:

* Capture clipboard updates
* Trigger UI via system shortcuts

---

### 3. UI Layer

Handles rendering and interaction.

* `HistoryModel` – Qt model
* `HistoryPopup` – popup interface

Responsibilities:

* Display clipboard history
* Handle user selection

---

## Design Principles

### Memory First

* Hard limits on number of items
* Per-item size caps
* Total memory budget enforcement
* No unbounded allocations

### Minimal Background Activity

* No polling loops
* No timers
* Purely event-driven

### Simplicity Over Features

* Start with text only
* Add complexity incrementally
* Avoid premature optimisation traps

---

## Build Instructions

### Requirements

* Qt 6 (Core, Gui, Widgets)
* CMake ≥ 3.20
* C++20 compiler

### Ubuntu/Debian

```bash
sudo apt install cmake g++ qt6-base-dev
```

### Build

```bash
cmake -S . -B build
cmake --build build -j
./build/clipit
```

---

## Project Structure

```text
ClipIt/
├── CMakeLists.txt
├── src/
│   ├── app/
│   ├── core/
│   ├── platform/
│   └── ui/
└── tests/
```

---

## Current Limitations

* Global shortcut requires manual binding on Wayland (see strategy above)
* Text-only clipboard support
* Basic UI (no styling or positioning logic)
* No persistence (in-memory only)

---

## Why This Project Exists

This is not just a utility—it is a **systems engineering exercise** focused on:

* Efficient data handling
* OS-level integration
* Scalable architecture design
* Real-world desktop application constraints

---

## Next Milestone

Implement **global shortcut trigger** using the two-backend strategy:

1. `GlobalHotkeySocket` — `QLocalServer` listening on `$XDG_RUNTIME_DIR/clipit.sock`
2. `GlobalHotkeyX11` — `XGrabKey` + `QAbstractNativeEventFilter`, activated only when `XDG_SESSION_TYPE=x11`
3. `GlobalHotkey::create()` factory — runtime selection, returns the appropriate backend

This introduces:

* Platform-specific abstraction with a clean interface
* Native X11 event handling
* Unix socket IPC for compositor-agnostic triggering

---

## Contribution Philosophy

Keep changes:

* Small
* Measurable
* Memory-aware

Avoid:

* Feature bloat
* Unbounded storage
* Background-heavy designs

---

## Status

Early-stage MVP.
Core architecture is in place; platform integration and optimisation in progress.
