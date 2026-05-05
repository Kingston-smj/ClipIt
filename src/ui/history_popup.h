#pragma once

#include <QWidget>

class QListView;

namespace ui {

class HistoryModel;

class HistoryPopup : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPopup(HistoryModel& model);

signals:
    void selected(const QString& text);

private:
    QListView* list_;
};

}
