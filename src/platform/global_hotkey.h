#pragma once

#include <QObject>
#include <memory>

namespace platform {

// Abstract interface for the global-hotkey trigger.
class GlobalHotkey : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject* parent = nullptr) : QObject(parent) {}
    ~GlobalHotkey() override = default;

    // Start listening.  Emits activated() when the shortcut fires.
    virtual void start() = 0;

    // Factory: creates the socket trigger used by desktop shortcuts.
    static std::unique_ptr<GlobalHotkey> create(QObject* parent = nullptr);

signals:
    void activated();
};

} // namespace platform
