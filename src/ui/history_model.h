#pragma once

#include <QAbstractListModel>
#include "core/clipboard_history.h"

namespace ui {

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit HistoryModel(core::ClipboardHistory& history);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void refresh();

private:
    core::ClipboardHistory& history_;
};

}
