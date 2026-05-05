#include "application_controller.h"
#include "core/clipboard_history.h"
#include "platform/clipboard_watcher_qt.h"
#include "ui/history_model.h"
#include "ui/history_popup.h"
#include <QGuiApplication>
#include <QClipboard>

namespace app {

ApplicationController::ApplicationController()
{
    history_ = std::make_unique<core::ClipboardHistory>();
    model_ = std::make_unique<ui::HistoryModel>(*history_);
    popup_ = std::make_unique<ui::HistoryPopup>(*model_);
    watcher_ = std::make_unique<platform::ClipboardWatcherQt>();

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::textCaptured,
                     [this](const QString& text){
        history_->push(text);
        model_->refresh();
    });

    QObject::connect(popup_.get(), &ui::HistoryPopup::selected,
                     [](const QString& text){
        QGuiApplication::clipboard()->setText(text);
    });
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::start()
{
    watcher_->start();
    popup_->show();
}

}
