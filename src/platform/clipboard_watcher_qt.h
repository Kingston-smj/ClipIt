#pragma once

#include <QObject>
#include <QImage>

namespace platform {

class ClipboardWatcherQt : public QObject
{
    Q_OBJECT

public:
    void start();
    void stop();
    bool isRunning() const { return running_; }

signals:
    void textCaptured(const QString& text);
    void imageCaptured(const QImage& image);

private:
    QMetaObject::Connection connection_;
    bool running_{false};
};

} // namespace platform
