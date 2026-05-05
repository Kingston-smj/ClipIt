#pragma once

#include <QObject>
#include <memory>

namespace core { class ClipboardHistory; }
namespace ui { class HistoryModel; class HistoryPopup; }
namespace platform { class ClipboardWatcherQt; class GlobalHotkey; }

namespace app {

class ApplicationController : public QObject
{
    Q_OBJECT

public:
    ApplicationController();
    ~ApplicationController();
    void start();

private:
    std::unique_ptr<core::ClipboardHistory> history_;
    std::unique_ptr<ui::HistoryModel> model_;
    std::unique_ptr<ui::HistoryPopup> popup_;
    std::unique_ptr<platform::ClipboardWatcherQt> watcher_;
};

}
