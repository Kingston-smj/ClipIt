#include <QApplication>
#include <QLocalSocket>
#include <QCoreApplication>
#include <cstdlib>
#include <cstdio>
#include "app/application_controller.h"

// --trigger mode: poke a running ClipIt instance via its socket and exit.
static int trigger()
{
    const char* xdg = std::getenv("XDG_RUNTIME_DIR");
    if (!xdg)
    {
        std::fprintf(stderr, "clipit --trigger: XDG_RUNTIME_DIR not set\n");
        return 1;
    }

    const QString path = QString::fromUtf8(xdg) + QStringLiteral("/clipit.sock");

    QLocalSocket sock;
    sock.connectToServer(path);

    if (!sock.waitForConnected(500))
    {
        std::fprintf(stderr, "clipit --trigger: could not connect to %s\n"
                             "  Is ClipIt running?\n", qPrintable(path));
        return 1;
    }

    sock.disconnectFromServer();
    return 0;
}

int main(int argc, char* argv[])
{
    // --trigger: no GUI needed, connect to socket and exit immediately.
    for (int i = 1; i < argc; ++i)
    {
        if (qstrcmp(argv[i], "--trigger") == 0)
        {
            QCoreApplication app(argc, argv);
            return trigger();
        }
    }

    QApplication app(argc, argv);
    app::ApplicationController controller;
    controller.start();
    return app.exec();
}
