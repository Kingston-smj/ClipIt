#include "application_controller.h"
#include "core/clipboard_history.h"
#include "core/clipboard_item.h"
#include "platform/clipboard_watcher_qt.h"
#include "platform/global_hotkey.h"
#include "platform/global_hotkey_socket.h"
#include "platform/gnome_shell_bridge.h"
#include "ui/history_model.h"
#include "ui/history_popup.h"
#include "app/logging.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QBuffer>
#include <QStandardPaths>
#include <QCryptographicHash>

namespace app {

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
    history_ = std::make_unique<core::ClipboardHistory>(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        QStringLiteral("/pinned"));
    model_   = std::make_unique<ui::HistoryModel>(*history_);
    popup_   = std::make_unique<ui::HistoryPopup>(*model_);
    watcher_ = std::make_unique<platform::ClipboardWatcherQt>();
    hotkey_  = platform::GlobalHotkey::create();
    shell_bridge_ = std::make_unique<platform::GnomeShellBridge>();

    // ── Clipboard capture ─────────────────────────────────────────────────────

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::textCaptured,
                     this, [this](const QString& text) {
        if (QGuiApplication::clipboard()->ownsClipboard() || text == suppressed_text_)
            return;
        suppressed_text_.clear();
        suppressed_png_hash_.clear();
        suppressed_png_size_ = 0;
        history_->push(text);
        model_->refresh();
    });

    QObject::connect(watcher_.get(), &platform::ClipboardWatcherQt::imageCaptured,
                     this, [this](const QImage& image) {
        if (QGuiApplication::clipboard()->ownsClipboard())
            return;
        suppressed_text_.clear();
        suppressed_png_hash_.clear();
        suppressed_png_size_ = 0;
        history_->push(image);
        model_->refresh();
    });

    QObject::connect(shell_bridge_.get(), &platform::GnomeShellBridge::textCaptured,
                     this, [this](const QString& text) {
        if (text == suppressed_text_)
            return;
        suppressed_text_.clear();
        suppressed_png_hash_.clear();
        suppressed_png_size_ = 0;
        history_->push(text);
        model_->refresh();
    });

    QObject::connect(shell_bridge_.get(), &platform::GnomeShellBridge::pngCaptured,
                     this, [this](const QByteArray& png) {
        if (!suppressed_png_hash_.isEmpty() &&
            png.size() == suppressed_png_size_ &&
            QCryptographicHash::hash(png, QCryptographicHash::Sha256) ==
                suppressed_png_hash_)
        {
            suppressed_png_hash_.clear();
            suppressed_png_size_ = 0;
            return;
        }
        suppressed_text_.clear();
        suppressed_png_hash_.clear();
        suppressed_png_size_ = 0;
        history_->pushPng(png);
        model_->refresh();
    });

    // GNOME Shell can observe clipboard changes that Qt cannot see while the
    // app is in the background. Keep exactly one listener active so one copy
    // produces one history item and images are not decoded twice.
    QObject::connect(shell_bridge_.get(),
                     &platform::GnomeShellBridge::availabilityChanged,
                     this, [this](bool available) {
        if (available)
            watcher_->stop();
        else
            watcher_->start();
    });

    // ── Clipboard restore ─────────────────────────────────────────────────────

    QObject::connect(popup_.get(), &ui::HistoryPopup::selected,
                     this, [this](const QString& id) {
        const core::ClipboardItem item = history_->itemForPaste(id);
        if (item.isText())
        {
            if (item.text.isEmpty())
                return;
            suppressed_text_ = item.text;
            suppressed_png_hash_.clear();
            suppressed_png_size_ = 0;
            QGuiApplication::clipboard()->setText(item.text);
        }
        else if (item.isImage())
        {
            if (item.png_data.isEmpty())
            {
                app::log_warn("ClipIt: selected image could not be read");
                return;
            }
            // Decompress PNG bytes back to QImage and set on clipboard.
            QImage image;
            if (!image.loadFromData(item.png_data, "PNG"))
            {
                app::log_warn("ClipIt: selected image could not be decoded");
                return;
            }
            suppressed_png_hash_ =
                QCryptographicHash::hash(item.png_data, QCryptographicHash::Sha256);
            suppressed_png_size_ = item.png_data.size();
            suppressed_text_.clear();

            auto* mime = new QMimeData();
            mime->setImageData(image);
            QGuiApplication::clipboard()->setMimeData(mime);
        }
        shell_bridge_->paste();
    });

    // ── Picker character paste ────────────────────────────────────────────────

    QObject::connect(popup_.get(), &ui::HistoryPopup::characterPasted,
                     this, [this](const QString& ch) {
        // Don't add the pasted character to clipboard history.
        suppressed_text_ = ch;
        suppressed_png_hash_.clear();
        suppressed_png_size_ = 0;
        QGuiApplication::clipboard()->setText(ch);
        shell_bridge_->paste();
    });

    // ── Global hotkey ─────────────────────────────────────────────────────────

    QObject::connect(hotkey_.get(), &platform::GlobalHotkey::activated,
                     this, &ApplicationController::showPopup);
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::start()
{
    shell_bridge_->start();
    if (!shell_bridge_->isExtensionAvailable())
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
    popup_->showPopup();
}

} // namespace app
