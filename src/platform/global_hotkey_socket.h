#pragma once

#include "global_hotkey.h"
#include <QLocalServer>

namespace platform {

// Primary trigger — works on every compositor (GNOME Wayland, KDE Wayland,
// Hyprland, Sway, X11).
//
// Listens on a Unix domain socket at:
//   $XDG_RUNTIME_DIR/clipit.sock
//
// Any incoming connection triggers activated().
// Wire up your DE shortcut (Super+V) to:
//   echo '' | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/clipit.sock
class GlobalHotkeySocket : public GlobalHotkey
{
    Q_OBJECT

public:
    explicit GlobalHotkeySocket(QObject* parent = nullptr);

    void start() override;

    // The full socket path used, so callers / docs can print it.
    QString socketPath() const;

private:
    QLocalServer* server_{nullptr};
    QString       socket_path_;
};

} // namespace platform
