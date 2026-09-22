#include "history_model.h"
#include <QPixmap>

namespace ui {

HistoryModel::HistoryModel(core::ClipboardHistory& history)
    : history_(history)
{}

int HistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(history_.items().size());
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(history_.items().size()))
        return {};

    const core::ClipboardItem& item = history_.items()[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
        if (item.isText())
            return item.text;
        // Show dimensions for images.
        return QStringLiteral("[Image %1×%2]")
            .arg(item.thumbnail.width() > 0 ? item.thumbnail.width()  : 0)
            .arg(item.thumbnail.height() > 0 ? item.thumbnail.height() : 0);

    case Qt::ToolTipRole:
        if (item.isText())
            return item.text;
        return QStringLiteral("Image · %1 KB compressed")
            .arg(item.byte_size / 1024);

    case Qt::DecorationRole:
        if (item.isImage() && !item.thumbnail.isNull())
            return QPixmap::fromImage(item.thumbnail);
        return {};

    case Qt::UserRole:
        // Full ClipboardItem for the popup to use when restoring to clipboard.
        return QVariant::fromValue(item);

    default:
        return {};
    }
}

void HistoryModel::refresh()
{
    beginResetModel();
    endResetModel();
}

} // namespace ui
