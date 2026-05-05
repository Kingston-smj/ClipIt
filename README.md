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

* Global shortcut (**Super + V**) on X11
* Wayland-compatible fallback trigger
* Support for:

  * HTML content
  * File paths (URI list)
  * Images (compressed storage)
* Search/filter inside history
* Persistent storage (optional, bounded)
* Tray integration + autostart
* Cross-platform support (Windows, macOS)

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
* `GlobalHotkey` – (stub, X11 implementation planned)

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

* No global hotkey yet
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

Implement **X11 global shortcut (Super + V)** using:

* `XGrabKey`
* `QAbstractNativeEventFilter`

This introduces:

* Native event handling
* Platform-specific abstraction
* Input system integration

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
