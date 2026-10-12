#pragma once

#include "clipboard_item.h"
#include "memory_policy.h"
#include <deque>
#include <QImage>
#include <QString>

namespace core {

class ClipboardHistory
{
public:
    explicit ClipboardHistory(QString storage_dir = {});
    void push(const QString& text);
    void push(const QImage&  image);
    void pushPng(const QByteArray& png);

    const std::deque<ClipboardItem>& items() const;
    const std::deque<ClipboardItem>& pinnedItems() const { return pinned_; }
    const ClipboardItem* find(const QString& id) const;
    ClipboardItem itemForPaste(const QString& id) const;
    bool pin(const QString& id);
    bool unpin(const QString& id);
    bool edit(const QString& id, const QString& value);
    bool remove(const QString& id);
    QString lastError() const { return last_error_; }

    qsizetype totalBytes() const { return total_bytes_; }

private:
    void pushItem(ClipboardItem item);
    void evictToFit(qsizetype incoming_bytes);
    void loadPinned();
    bool savePinned(const std::deque<ClipboardItem>& candidate);
    QString imagePath(const QString& id) const;
    QByteArray loadImage(const ClipboardItem& item) const;
    void setError(const QString& error);

    std::deque<ClipboardItem> data_;
    std::deque<ClipboardItem> pinned_;
    qsizetype                 total_bytes_{0};
    QString storage_dir_;
    QString last_error_;
};

} // namespace core
