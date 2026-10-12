#pragma once

#include <QAbstractListModel>
#include "core/clipboard_history.h"

namespace ui {

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit HistoryModel(core::ClipboardHistory& history);

    int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void refresh();
    const core::ClipboardItem* find(const QString& id) const;
    bool pin(const QString& id);
    bool unpin(const QString& id);
    bool edit(const QString& id, const QString& value);
    bool remove(const QString& id);
    QString lastError() const;

private:
    core::ClipboardHistory& history_;
};

} // namespace ui
