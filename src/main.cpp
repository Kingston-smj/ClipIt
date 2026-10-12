#include <QApplication>
#include <QLocalSocket>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <cstdlib>
#include <cstdio>
#include "app/application_controller.h"
#include "app/logging.h"

// --trigger mode: poke a running ClipIt instance via its socket and exit.
// If no instance is running, start one in the background first.
static int trigger(const QString& selfPath)
{
    const char* xdg = std::getenv("XDG_RUNTIME_DIR");
    if (!xdg)
    {
        app::log_error("clipit --trigger: XDG_RUNTIME_DIR not set");
        return 1;
    }

    const QString runtimeDir = QString::fromUtf8(xdg);
    const QByteArray logPath =
        (runtimeDir + QStringLiteral("/clipit-trigger-token.log")).toLocal8Bit();
    if (std::FILE* log = std::fopen(logPath.constData(), "w"))
    {
        const char* token = std::getenv("XDG_ACTIVATION_TOKEN");
        std::fprintf(log, "XDG_ACTIVATION_TOKEN=%s\n", token ? token : "(unset)");
        std::fclose(log);
    }

    const QString path = runtimeDir + QStringLiteral("/clipit.sock");

    QLocalSocket sock;
    sock.connectToServer(path);

    if (!sock.waitForConnected(300))
    {
        // No daemon running — start one detached, then give it a moment to bind.
        if (!QProcess::startDetached(selfPath, {}))
        {
            app::log_error("clipit --trigger: failed to start daemon");
            return 1;
        }

        // Wait up to 1 s for the daemon to bind the socket.
        for (int i = 0; i < 10; ++i)
        {
            QThread::msleep(100);
            sock.connectToServer(path);
            if (sock.waitForConnected(200))
                break;
        }

        if (sock.state() != QLocalSocket::ConnectedState)
        {
            app::log_error("clipit --trigger: daemon started but socket not ready");
            return 1;
        }
    }

    sock.disconnectFromServer();
    return 0;
}

int main(int argc, char* argv[])
{
    app::install_logging();

    // --trigger: no GUI needed, connect to socket and exit immediately.
    for (int i = 1; i < argc; ++i)
    {
        if (qstrcmp(argv[i], "--trigger") == 0)
        {
            QCoreApplication app(argc, argv);
            // Pass our own path so trigger() can restart the daemon if needed.
            return trigger(QString::fromLocal8Bit(argv[0]));
        }
    }

    QApplication app(argc, argv);
    app::ApplicationController controller;
    controller.start();
    return app.exec();
}
