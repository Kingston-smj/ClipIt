#include "clipboard_history.h"

namespace core {

void ClipboardHistory::push(const QString& text)
{
    if (!data_.empty() && data_.front() == text)
        return;

    data_.insert(data_.begin(), text);
    if (data_.size() > MAX)
        data_.pop_back();
}

const std::vector<QString>& ClipboardHistory::items() const
{
    return data_;
}

}
