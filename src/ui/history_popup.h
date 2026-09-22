#pragma once

#include <QWidget>

class QListView;
class QKeyEvent;

namespace ui {

class HistoryModel;

class HistoryPopup : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPopup(HistoryModel& model, QWidget* parent = nullptr);

    void showAtTopLeft();

signals:
    void selected(const QString& text);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QListView* list_;
};

}

