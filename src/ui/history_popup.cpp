#include "history_popup.h"
#include "history_model.h"
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

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(list_);

    auto handleSelection = [this](const QModelIndex& idx){
        if (idx.isValid())
        {
            emit selected(idx.data().toString());
            hide();
        }
    };

    connect(list_, &QListView::clicked, handleSelection);
    connect(list_, &QListView::activated, handleSelection);
}

void HistoryPopup::showAtTopLeft()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect screenRect = screen->availableGeometry();
        // Position at top-left with a clean 20px padding
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

}

