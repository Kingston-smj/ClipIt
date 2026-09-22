#include "global_hotkey_socket.h"
#include "app/logging.h"
#include <QLocalSocket>
#include <QStandardPaths>
#include <QDir>
#include <cstdlib>

namespace platform {

GlobalHotkeySocket::GlobalHotkeySocket(QObject* parent)
    : GlobalHotkey(parent)
    , server_(new QLocalServer(this))
{
    // Prefer $XDG_RUNTIME_DIR (per-user, tmpfs); fall back to a writable temp dir.
    const char* xdg = std::getenv("XDG_RUNTIME_DIR");
    const QString base = xdg ? QString::fromUtf8(xdg)
                              : QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    socket_path_ = base + QStringLiteral("/clipit.sock");
}

void GlobalHotkeySocket::start()
{
    // Remove any leftover socket from a previous crash.
    QLocalServer::removeServer(socket_path_);

    if (!server_->listen(socket_path_))
    {
        app::log_warn("GlobalHotkeySocket: could not listen on %s — %s",
                      qPrintable(socket_path_),
                      qPrintable(server_->errorString()));
        return;
    }

    app::log_info("GlobalHotkeySocket: listening on %s", qPrintable(socket_path_));

    // Each incoming connection = one trigger; drain and close immediately.
    QObject::connect(server_, &QLocalServer::newConnection, this, [this]() {
        if (QLocalSocket* conn = server_->nextPendingConnection())
        {
            conn->close();
            conn->deleteLater();
        }
        emit activated();
    });
}

QString GlobalHotkeySocket::socketPath() const
{
    return socket_path_;
}

} // namespace platform
