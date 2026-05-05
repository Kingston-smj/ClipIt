#include "history_popup.h"
#include "history_model.h"
#include <QListView>
#include <QVBoxLayout>
#include <QModelIndex>

namespace ui {

HistoryPopup::HistoryPopup(HistoryModel& model)
{
    list_ = new QListView(this);
    list_->setModel(&model);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(list_);

    connect(list_, &QListView::clicked, [this](const QModelIndex& idx){
        emit selected(idx.data().toString());
    });
}

}
