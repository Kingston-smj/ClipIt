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
    explicit ApplicationController(QObject* parent = nullptr);
    ~ApplicationController() override;
    void start();
    void showPopup();

private:
    std::unique_ptr<core::ClipboardHistory>      history_;
    std::unique_ptr<ui::HistoryModel>             model_;
    std::unique_ptr<ui::HistoryPopup>             popup_;
    std::unique_ptr<platform::ClipboardWatcherQt> watcher_;
    std::unique_ptr<platform::GlobalHotkey>       hotkey_;
    bool ignore_next_clipboard_change_{false};
};

}

