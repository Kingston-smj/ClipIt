#pragma once

#include <QByteArray>
#include <QDBusContext>
#include <QObject>
#include <QString>

namespace platform {

// Only the GNOME Shell extension uses this session-bus endpoint. The Qt
// watcher remains available on other desktops.
class GnomeShellBridge : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.clipit.Bridge")

public:
    explicit GnomeShellBridge(QObject* parent = nullptr);
    void start();
    void paste();

public slots:
    void CaptureText(const QString& text);
    void CapturePng(const QByteArray& png);

signals:
    void textCaptured(const QString& text);
    void pngCaptured(const QByteArray& png);

private:
    bool isShellSender() const;
    bool active_{false};
    bool missing_extension_logged_{false};
};

} // namespace platform
