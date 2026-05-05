#include "history_model.h"

namespace ui {

HistoryModel::HistoryModel(core::ClipboardHistory& history)
    : history_(history)
{
}

int HistoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(history_.items().size());
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(history_.items().size()))
        return {};

    if (role == Qt::DisplayRole)
        return history_.items()[index.row()];

    return {};
}

void HistoryModel::refresh()
{
    beginResetModel();
    endResetModel();
}

}
