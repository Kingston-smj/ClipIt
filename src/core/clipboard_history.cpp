#include "clipboard_history.h"
#include <algorithm>

namespace core {

// ── Public push overloads ─────────────────────────────────────────────────────

void ClipboardHistory::push(const QString& text)
{
    if (text.isEmpty())
        return;

    auto item = ClipboardItem::fromText(text);

    if (item.byte_size > MemoryPolicy::MAX_TEXT_BYTES)
        return; // single item over per-type cap — silently drop

    pushItem(std::move(item));
}

void ClipboardHistory::push(const QImage& image)
{
    if (image.isNull())
        return;

    auto item = ClipboardItem::fromImage(image);

    if (item.byte_size > MemoryPolicy::MAX_IMAGE_BYTES)
        return; // compressed image still over cap — drop

    pushItem(std::move(item));
}

// ── Internal ──────────────────────────────────────────────────────────────────

void ClipboardHistory::pushItem(ClipboardItem item)
{
    // MRU promotion: if identical item already exists, remove it first.
    auto it = std::find(data_.begin(), data_.end(), item);
    if (it != data_.end())
    {
        total_bytes_ -= it->byte_size;
        data_.erase(it);
    }

    // Evict oldest entries until the new item fits within the total budget.
    evictToFit(item.byte_size);

    total_bytes_ += item.byte_size;
    data_.push_front(std::move(item));
}

void ClipboardHistory::evictToFit(qsizetype incoming_bytes)
{
    // Evict from the back (oldest) until both the count and byte limits are met.
    while (!data_.empty() &&
           (data_.size() >= MemoryPolicy::MAX_ITEMS ||
            total_bytes_ + incoming_bytes > MemoryPolicy::MAX_TOTAL_BYTES))
    {
        total_bytes_ -= data_.back().byte_size;
        data_.pop_back();
    }
}

// ── Accessors ─────────────────────────────────────────────────────────────────

const std::deque<ClipboardItem>& ClipboardHistory::items() const
{
    return data_;
}

} // namespace core
