#include "clipboard_watcher_qt.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>

namespace platform {

void ClipboardWatcherQt::start()
{
    auto* cb = QGuiApplication::clipboard();

    QObject::connect(cb, &QClipboard::dataChanged, [this, cb] {
        const QMimeData* mime = cb->mimeData();
        if (!mime)
            return;

        // Images take priority — a screenshot copy often also has text metadata.
        if (mime->hasImage())
        {
            QImage image = qvariant_cast<QImage>(mime->imageData());
            if (!image.isNull())
            {
                emit imageCaptured(image);
                return;
            }
        }

        if (mime->hasText())
        {
            QString text = mime->text();
            if (!text.isEmpty())
                emit textCaptured(text);
        }
    });
}

} // namespace platform
