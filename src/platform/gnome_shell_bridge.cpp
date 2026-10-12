#include "gnome_shell_bridge.h"
#include "gnome_shell_probe.h"
#include "app/logging.h"
#include "core/memory_policy.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QTimer>

namespace platform {
namespace {
constexpr auto kClipItService = "org.clipit.ClipIt";
constexpr auto kClipItPath = "/org/clipit/Bridge";
constexpr auto kShellService = "org.gnome.Shell";
constexpr auto kShellPath = "/org/clipit/Shell";
constexpr auto kShellInterface = "org.clipit.Shell";

}

GnomeShellBridge::GnomeShellBridge(QObject* parent) : QObject(parent) {}

void GnomeShellBridge::start()
{
    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland")))
        return;

    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || !bus.registerService(QLatin1String(kClipItService)) ||
        !bus.registerObject(QLatin1String(kClipItPath), this,
                            QDBusConnection::ExportAllSlots))
    {
        app::log_warn("GnomeShellBridge: cannot register session-bus endpoint");
        return;
    }
    registered_ = true;
    app::log_info("GnomeShellBridge: listening for GNOME clipboard events");

    // Introspection works with the installed extension as well as newer
    // versions. The new extension also sends BridgeReady/BridgeStopping.
    auto inspect = QDBusMessage::createMethodCall(
        QLatin1String(kShellService), QLatin1String(kShellPath),
        QStringLiteral("org.freedesktop.DBus.Introspectable"),
        QStringLiteral("Introspect"));
    const QDBusMessage reply = bus.call(inspect, QDBus::Block, 250);
    setExtensionAvailable(
        reply.type() == QDBusMessage::ReplyMessage &&
        reply.arguments().size() == 1 &&
        hasShellPasteMethod(reply.arguments().front().toString()));
}

bool GnomeShellBridge::isShellSender() const
{
    if (!calledFromDBus())
        return false;
    const auto bus = QDBusConnection::sessionBus();
    const auto owner = bus.interface()->serviceOwner(QLatin1String(kShellService));
    return owner.isValid() && message().service() == owner.value();
}

void GnomeShellBridge::CaptureText(const QString& text)
{
    if (!extension_available_ || !isShellSender() || text.isEmpty() ||
        text.size() * qsizetype(sizeof(QChar)) > core::MemoryPolicy::MAX_TEXT_BYTES)
        return;
    emit textCaptured(text);
}

void GnomeShellBridge::CapturePng(const QByteArray& png)
{
    if (!extension_available_ || !isShellSender() || png.isEmpty() ||
        png.size() > core::MemoryPolicy::MAX_IMAGE_BYTES)
        return;
    emit pngCaptured(png);
}

void GnomeShellBridge::BridgeReady()
{
    if (isShellSender())
        setExtensionAvailable(true);
}

void GnomeShellBridge::BridgeStopping()
{
    if (isShellSender())
        setExtensionAvailable(false);
}

void GnomeShellBridge::setExtensionAvailable(bool available)
{
    if (extension_available_ == available)
        return;
    extension_available_ = available;
    app::log_info("GnomeShellBridge: Shell extension %s",
                  available ? "available" : "unavailable");
    emit availabilityChanged(available);
}

void GnomeShellBridge::paste()
{
    if (!registered_ || !extension_available_)
        return;

    // Defer until the selection handler has hidden the popup. GNOME Shell
    // adds one short delay for focus to return before sending Ctrl+V.
    QTimer::singleShot(0, this, [this] {
        auto call = QDBusMessage::createMethodCall(QLatin1String(kShellService),
            QLatin1String(kShellPath), QLatin1String(kShellInterface),
            QStringLiteral("Paste"));
        auto* watcher = new QDBusPendingCallWatcher(
            QDBusConnection::sessionBus().asyncCall(call), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this, watcher] {
            QDBusPendingReply<> reply = *watcher;
            watcher->deleteLater();
            if (reply.isError() && !missing_extension_logged_)
            {
                missing_extension_logged_ = true;
                app::log_warn("GnomeShellBridge: Shell extension unavailable; item remains on clipboard (%s)",
                              qPrintable(reply.error().message()));
            }
            else if (!reply.isError())
                missing_extension_logged_ = false;
        });
    });
}

} // namespace platform
