#pragma once

#include <QWidget>
#include "core/clipboard_item.h"

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
    // Emits the full ClipboardItem so the controller can restore
    // both text and images to the system clipboard correctly.
    void selected(const core::ClipboardItem& item);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QListView* list_;
};

} // namespace ui
