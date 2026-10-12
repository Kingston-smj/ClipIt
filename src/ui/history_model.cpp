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
    return static_cast<int>(history_.pinnedItems().size() + history_.items().size());
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return {};

    const size_t row = size_t(index.row());
    const auto& pins = history_.pinnedItems();
    const core::ClipboardItem& item = row < pins.size()
        ? pins[row] : history_.items()[row - pins.size()];

    switch (role)
    {
    case Qt::DisplayRole:
        if (item.isText())
            return (item.pinned ? QStringLiteral("📌 ") : QString()) + item.text;
        {
            const QString note = item.note.simplified();
            const QString label = note.isEmpty() ? QStringLiteral("[Image %1×%2]")
                .arg(item.thumbnail.width()).arg(item.thumbnail.height()) :
                note.left(120) + (note.size() > 120 ? QStringLiteral("…") : QString());
            return (item.pinned ? QStringLiteral("📌 ") : QString()) + label;
        }

    case Qt::ToolTipRole:
        if (item.isText())
            return item.text;
        return QStringLiteral("Image %1×%2 · %3 KB compressed%4")
            .arg(item.thumbnail.width()).arg(item.thumbnail.height())
            .arg(item.byte_size / 1024)
            .arg(item.note.isEmpty() ? QString() : QStringLiteral("\n") + item.note);

    case Qt::DecorationRole:
        if (item.isImage() && !item.thumbnail.isNull())
            return QPixmap::fromImage(item.thumbnail);
        return {};

    case Qt::UserRole:
        return item.id;

    case Qt::UserRole + 1:
        return item.pinned;

    default:
        return {};
    }
}

const core::ClipboardItem* HistoryModel::find(const QString& id) const
{
    return history_.find(id);
}

bool HistoryModel::pin(const QString& id)
{
    if (!history_.pin(id)) return false;
    refresh();
    return true;
}

bool HistoryModel::unpin(const QString& id)
{
    if (!history_.unpin(id)) return false;
    refresh();
    return true;
}

bool HistoryModel::edit(const QString& id, const QString& value)
{
    if (!history_.edit(id, value)) return false;
    refresh();
    return true;
}

bool HistoryModel::remove(const QString& id)
{
    if (!history_.remove(id)) return false;
    refresh();
    return true;
}

QString HistoryModel::lastError() const
{
    return history_.lastError();
}

void HistoryModel::refresh()
{
    beginResetModel();
    endResetModel();
}

} // namespace ui
