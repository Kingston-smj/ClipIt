#pragma once

#include <QString>
#include <deque>

namespace core {

class ClipboardHistory
{
public:
    void push(const QString& text);
    const std::deque<QString>& items() const;

private:
    std::deque<QString> data_;
    static constexpr size_t MAX = 10;
};

}

