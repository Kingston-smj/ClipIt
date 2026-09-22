#pragma once

#include "global_hotkey.h"
#include <QAbstractNativeEventFilter>

// Only compiled when building for X11.
// Guarded by the CMake target property X11_BACKEND.

namespace platform {

// Secondary trigger — X11 / XWayland sessions only.
//
// Grabs Super+V system-wide via XGrabKey.  Installs a QAbstractNativeEventFilter
// to intercept KeyPress XCB events before Qt sees them.
//
// On Wayland (XDG_SESSION_TYPE != "x11") the factory will not instantiate this
// class — use GlobalHotkeySocket instead.
class GlobalHotkeyX11 : public GlobalHotkey, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit GlobalHotkeyX11(QObject* parent = nullptr);
    ~GlobalHotkeyX11() override;

    void start() override;

    // QAbstractNativeEventFilter
    bool nativeEventFilter(const QByteArray& eventType,
                           void*             message,
                           qintptr*          result) override;

private:
    void grabKey();
    void ungrabKey();

    unsigned int keycode_{0};
    unsigned int modifiers_{0};
    bool         grabbed_{false};
};

} // namespace platform
