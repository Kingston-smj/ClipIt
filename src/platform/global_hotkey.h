#pragma once

#include <QObject>
#include <memory>

namespace platform {

// Abstract interface for all global-hotkey backends.
class GlobalHotkey : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject* parent = nullptr) : QObject(parent) {}
    ~GlobalHotkey() override = default;

    // Start listening.  Emits activated() when the shortcut fires.
    virtual void start() = 0;

    // Factory: picks the right backend at runtime.
    //   - X11 session  → GlobalHotkeyX11  (XGrabKey, then falls back to socket)
    //   - Wayland/other → GlobalHotkeySocket only
    static std::unique_ptr<GlobalHotkey> create(QObject* parent = nullptr);

signals:
    void activated();
};

} // namespace platform
