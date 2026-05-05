#pragma once

#include <QObject>

namespace platform {

class ClipboardWatcherQt : public QObject
{
    Q_OBJECT

public:
    void start();

signals:
    void textCaptured(const QString& text);
};

}
