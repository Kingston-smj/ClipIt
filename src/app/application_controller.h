#pragma once

#include <QObject>
#include <memory>

namespace core { class ClipboardHistory; }
namespace ui { class HistoryModel; class HistoryPopup; }
namespace platform { class ClipboardWatcherQt; class GlobalHotkey; class GnomeShellBridge; }

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
    std::unique_ptr<platform::GnomeShellBridge>   shell_bridge_;
    QString suppressed_text_;
    QByteArray suppressed_png_;
};

}
