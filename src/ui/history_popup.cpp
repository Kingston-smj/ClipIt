#include "history_popup.h"
#include "history_model.h"
#include "core/clipboard_item.h"
#include <QListView>
#include <QVBoxLayout>
#include <QModelIndex>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>

namespace ui {

HistoryPopup::HistoryPopup(HistoryModel& model, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    resize(340, 420);

    list_ = new QListView(this);
    list_->setModel(&model);
    list_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Show thumbnails at a comfortable size.
    list_->setIconSize(QSize(64, 64));
    list_->setSpacing(2);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(list_);

    auto handleSelection = [this](const QModelIndex& idx) {
        if (!idx.isValid())
            return;

        // Retrieve the full ClipboardItem from UserRole.
        auto item = idx.data(Qt::UserRole).value<core::ClipboardItem>();
        emit selected(item);
        hide();
    };

    connect(list_, &QListView::clicked,   handleSelection);
    connect(list_, &QListView::activated, handleSelection);
}

void HistoryPopup::showAtTopLeft()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect screenRect = screen->availableGeometry();
        move(screenRect.topLeft() + QPoint(20, 20));
    }
    show();
    raise();
    activateWindow();
    list_->setFocus();
}

void HistoryPopup::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }
    QWidget::keyPressEvent(event);
}

} // namespace ui
