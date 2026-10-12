# ClipIt

A lightweight Linux clipboard manager built with Qt/C++ that brings a **Win + V**-style clipboard history to Wayland, with a strong focus on **memory efficiency** and **minimal background overhead**.

---

## Overview

ClipIt monitors your system clipboard, stores a bounded history of copied items, and allows quick access via a popup interface.

The project is designed as a **systems-level learning exercise** with production-oriented architecture:

* Clean separation of concerns (Core / Platform / UI)
* Event-driven clipboard monitoring
* Bounded clipboard-history storage
* Cross-platform extensibility

---

## Current Features

* Clipboard monitoring (text and image support)
* Fixed-size history (default: 10 items)
* Right-click Pin, Edit, and Delete for clipboard entries
* Up to 10 pinned entries persist across restarts; images can have an editable note
* Duplicate prevention
* Tabbed popup UI with dark theme (Catppuccin Mocha)
* Emoji, kaomoji, and symbol pickers (curated compile-time data)
* Single-click an item in any tab to restore it to the clipboard
* Self-healing global shortcut trigger
* Qt Model/View architecture

---

## Planned Features

* Search/filter inside history
* Support for:
  * HTML content
  * File paths (URI list)
* Optional persistence settings for users who want more than pinned items saved
* Tray integration + autostart
* Cross-platform support (Windows, macOS)

---

## Global Shortcut Strategy

ClipIt uses a Unix socket trigger for desktop shortcuts.

### Unix socket trigger

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

Trigger command (copy this into your shortcut):

```bash
clipit --trigger
```

*(Note: `clipit --trigger` is self-healing. If the background daemon isn't running, it will automatically start it before showing the popup.)*

This approach works on **GNOME Wayland, KDE Wayland, Hyprland, Sway** — any
setup that can launch a shell command from a key press.

---

## Architecture

### 1. Core (Platform-Agnostic)

Handles all memory-sensitive logic.

* `ClipboardHistory` – bounded, most-recent-first history
* `ClipboardItem` – compact data representation
* `MemoryPolicy` – limits for items and bytes

Responsibilities:

* Store clipboard entries
* Enforce memory constraints
* Deduplicate entries

---

### 2. Platform Layer

Handles OS-specific interactions.

* `ClipboardWatcherQt` – listens for clipboard changes on desktops that expose them
* `GnomeShellBridge` – receives GNOME Wayland copies and requests paste through
  a small GNOME Shell extension; no Remote Desktop session
* `GlobalHotkeySocket` – Unix socket listener for desktop shortcuts
* `GlobalHotkey::create()` – creates the socket trigger

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
* Total stored-payload budget enforcement
* Pre-scaled image thumbnails to avoid repeated full-size preview work
* Pinned image payloads stay on disk until selected for paste

### Minimal Background Activity

* Clipboard changes are received through Qt or GNOME Shell events
* No recurring background polling loop
* The popup and picker data are created once and reused

### Simplicity Over Features

* Add features incrementally
* Keep history and per-item storage bounded
* Avoid premature optimisation traps

---

## Build Instructions

### Requirements

* Qt 6 (Core, Gui, Widgets, Network, DBus)
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
* Recent, unpinned history is in memory only
* On GNOME Wayland, complete background history and one-click paste require the
  bundled Shell extension. It declares GNOME Shell 50 support; other major
  versions must be tested before adding them to its metadata.

### GNOME Wayland bridge (development install)

After building ClipIt, package the extension and install it for the current
user. On GNOME Wayland, log out and back in after the first install so Shell
discovers the extension, then enable it. Later extension updates also need a
new login to load the updated code.
The existing Super+V shortcut still runs `clipit --trigger`.

```bash
gnome-extensions pack --out-dir build \
  gnome-shell/clipit@kingston-smj.github.io
gnome-extensions install --force \
  build/clipit@kingston-smj.github.io.shell-extension.zip
```

After logging back in:

```bash
gnome-extensions enable clipit@kingston-smj.github.io
```

The extension listens to GNOME's selection-change signal and sends only text
up to 64 KiB or PNG data up to 4 MiB to the running ClipIt daemon over the
user's session bus. The normal 10-item, 32 MiB history limit still applies.
The extension uses a one-shot paste timer after a selection; it does no
background polling. If it is unavailable, the selected item remains on the
clipboard for manual paste.
On GNOME Wayland, ClipIt uses the Shell bridge as its only clipboard listener
when that extension is available. Otherwise it uses Qt's clipboard watcher.
This avoids handling the same image through both paths. The extension can
notify ClipIt when it is enabled or disabled while ClipIt is running.

### Pinned items

Right-click a history entry to Pin, Edit, or Delete it. Pinned entries stay
above recent history and survive ClipIt restarts. Edit changes a text entry's
content or adds a note shown beside an image thumbnail; selecting an image
still pastes the image alone. Unpin moves an entry back to recent history.

Only explicitly pinned entries are saved, under ClipIt's per-user application
data directory (normally `~/.local/share/clipit/pinned` on Linux). Text and
image notes are stored in a small manifest; each pinned image has its own PNG
file. The files are local and are not encrypted, so avoid pinning secrets on a
device shared with other accounts. The pinned store is limited to 10 entries
and 16 MiB of content; notes are limited to 1 KiB. Pinned images keep only
their thumbnail and note in memory until selected. Saves happen on a menu
action, with no background polling or disk write for ordinary copies.

---

## Why This Project Exists

This is not just a utility—it is a **systems engineering exercise** focused on:

* Efficient data handling
* OS-level integration
* Scalable architecture design
* Real-world desktop application constraints

---

## Next Milestone

Implement **Search and Filtering** or broader optional persistence:

* Allow users to filter their clipboard history using the same pattern implemented for the character pickers.
* Optionally persist recent history as well as the explicitly pinned entries.

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
