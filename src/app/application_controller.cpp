#include "application_controller.h"
#include "core/clipboard_history.h"
#include "core/clipboard_item.h"
#include "platform/clipboard_watcher_qt.h"
#include "platform/global_hotkey.h"
#include "platform/global_hotkey_socket.h"
#include "ui/history_model.h"
#include "ui/history_popup.h"
#include "app/logging.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QBuffer>

namespace app {

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
    history_ = std::make_unique<core::ClipboardHistory>();
    model_   = std::make_unique<ui::HistoryModel>(*history_);
    popup_   = std::make_unique<ui::HistoryPopup>(*model_);
    watcher_ = std::make_unique<platform::ClipboardWatcherQt>();
    hotkey_  = platform::GlobalHotkey::create();

    // ── Clipboard capture ─────────────────────────────────────────────────────

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::textCaptured,
                     this, [this](const QString& text) {
        if (ignore_next_clipboard_change_)
        {
            ignore_next_clipboard_change_ = false;
            return;
        }
        history_->push(text);
        model_->refresh();
    });

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::imageCaptured,
                     this, [this](const QImage& image) {
        if (ignore_next_clipboard_change_)
        {
            ignore_next_clipboard_change_ = false;
            return;
        }
        history_->push(image);
        model_->refresh();
    });

    // ── Clipboard restore ─────────────────────────────────────────────────────

    QObject::connect(popup_.get(), &ui::HistoryPopup::selected,
                     this, [this](const core::ClipboardItem& item) {
        ignore_next_clipboard_change_ = true;

        if (item.isText())
        {
            QGuiApplication::clipboard()->setText(item.text);
        }
        else if (item.isImage())
        {
            // Decompress PNG bytes back to QImage and set on clipboard.
            QImage image;
            image.loadFromData(item.png_data, "PNG");

            auto* mime = new QMimeData();
            mime->setImageData(image);
            QGuiApplication::clipboard()->setMimeData(mime);
        }
    });

    // ── Global hotkey ─────────────────────────────────────────────────────────

    QObject::connect(hotkey_.get(), &platform::GlobalHotkey::activated,
                     this, &ApplicationController::showPopup);
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::start()
{
    watcher_->start();
    hotkey_->start();

    if (auto* sock = dynamic_cast<platform::GlobalHotkeySocket*>(hotkey_.get()))
    {
        app::log_info("Trigger popup with: clipit --trigger");
        app::log_info("  (socket: %s)", qPrintable(sock->socketPath()));
    }
}

void ApplicationController::showPopup()
{
    popup_->showAtTopLeft();
}

} // namespace app
