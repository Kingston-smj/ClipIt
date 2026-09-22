#pragma once

#include "clipboard_item.h"
#include "memory_policy.h"
#include <deque>
#include <QImage>

namespace core {

class ClipboardHistory
{
public:
    void push(const QString& text);
    void push(const QImage&  image);

    const std::deque<ClipboardItem>& items() const;

    qsizetype totalBytes() const { return total_bytes_; }

private:
    void pushItem(ClipboardItem item);
    void evictToFit(qsizetype incoming_bytes);

    std::deque<ClipboardItem> data_;
    qsizetype                 total_bytes_{0};
};

} // namespace core
