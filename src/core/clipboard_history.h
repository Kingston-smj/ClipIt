#pragma once

#include <QString>
#include <vector>

namespace core {

class ClipboardHistory
{
public:
    void push(const QString& text);
    const std::vector<QString>& items() const;

private:
    std::vector<QString> data_;
    static constexpr size_t MAX = 10;
};

}
