#include "clipboard_history.h"
#include <algorithm>

namespace core {

void ClipboardHistory::push(const QString& text)
{
    if (text.isEmpty())
        return;

    auto it = std::find(data_.begin(), data_.end(), text);
    if (it != data_.end())
    {
        data_.erase(it);
    }

    data_.push_front(text);

    if (data_.size() > MAX)
    {
        data_.pop_back();
    }
}

const std::deque<QString>& ClipboardHistory::items() const
{
    return data_;
}

}

