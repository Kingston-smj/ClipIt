#pragma once

#include <QObject>
#include <QImage>

namespace platform {

class ClipboardWatcherQt : public QObject
{
    Q_OBJECT

public:
    void start();

signals:
    void textCaptured(const QString& text);
    void imageCaptured(const QImage& image);
};

} // namespace platform
