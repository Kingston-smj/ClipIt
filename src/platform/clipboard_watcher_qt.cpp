#include "clipboard_watcher_qt.h"
#include <QGuiApplication>
#include <QClipboard>

namespace platform {

void ClipboardWatcherQt::start()
{
    auto cb = QGuiApplication::clipboard();
    QObject::connect(cb, &QClipboard::dataChanged, [this, cb]{
        QString text = cb->text();
        if (!text.isEmpty())
            emit textCaptured(text);
    });
}

}
