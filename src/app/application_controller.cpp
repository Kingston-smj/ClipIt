#include "application_controller.h"
#include "core/clipboard_history.h"
#include "platform/clipboard_watcher_qt.h"
#include "platform/global_hotkey.h"
#include "platform/global_hotkey_socket.h"
#include "ui/history_model.h"
#include "ui/history_popup.h"
#include "app/logging.h"
#include <QGuiApplication>
#include <QClipboard>

namespace app {

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
    history_ = std::make_unique<core::ClipboardHistory>();
    model_   = std::make_unique<ui::HistoryModel>(*history_);
    popup_   = std::make_unique<ui::HistoryPopup>(*model_);
    watcher_ = std::make_unique<platform::ClipboardWatcherQt>();
    hotkey_  = platform::GlobalHotkey::create();

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::textCaptured,
                     this, [this](const QString& text){
        if (ignore_next_clipboard_change_)
        {
            ignore_next_clipboard_change_ = false;
            return;
        }
        history_->push(text);
        model_->refresh();
    });

    QObject::connect(popup_.get(), &ui::HistoryPopup::selected,
                     this, [this](const QString& text){
        ignore_next_clipboard_change_ = true;
        QGuiApplication::clipboard()->setText(text);
    });

    QObject::connect(hotkey_.get(), &platform::GlobalHotkey::activated,
                     this, &ApplicationController::showPopup);
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::start()
{
    watcher_->start();
    hotkey_->start();

    // On Wayland, print the socket path so the user knows what to bind.
    if (auto* sock = dynamic_cast<platform::GlobalHotkeySocket*>(hotkey_.get()))
    {
        app::log_info("Trigger popup with: echo '' | socat - UNIX-CONNECT:%s",
                      qPrintable(sock->socketPath()));
    }
}

void ApplicationController::showPopup()
{
    popup_->showAtTopLeft();
}

}

