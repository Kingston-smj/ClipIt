#include "history_popup.h"
#include "history_model.h"
#include "emoji_panel.h"
#include "kaomoji_panel.h"
#include "symbol_panel.h"
#include "core/clipboard_item.h"

#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QModelIndex>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

namespace ui {

// ─────────────────────────────────────────────────────────────────────────────
// Tab-bar layout constants
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int kPopupWidth   = 360;
static constexpr int kPopupHeight  = 480;
static constexpr int kTabBarHeight = 54;

struct TabDef {
    const char* icon;   // Unicode character used as the button label
    const char* tip;    // Tooltip shown on hover
};

static constexpr TabDef kTabs[] = {
    {"📋", "Clipboard history"},
    {"😊", "Emoji"},
    {";-)", "Kaomoji"},
    {"Ω",   "Symbols"},
};

// ─────────────────────────────────────────────────────────────────────────────
// HistoryPopup
// ─────────────────────────────────────────────────────────────────────────────

HistoryPopup::HistoryPopup(HistoryModel& model, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setObjectName("HistoryPopup");
    resize(kPopupWidth, kPopupHeight);
    applyStyleSheet();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Stacked pages ────────────────────────────────────────────────────
    pages_ = new QStackedWidget(this);

    // Page 0 — Clipboard history
    {
        auto* page = new QWidget();
        auto* pl   = new QVBoxLayout(page);
        pl->setContentsMargins(8, 8, 8, 4);

        list_ = new QListView(page);
        list_->setModel(&model);
        list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        list_->setIconSize(QSize(64, 64));
        list_->setSpacing(2);
        pl->addWidget(list_);

        pages_->addWidget(page);   // index 0 — TabHistory
    }

    // Pages 1-3 — pickers
    emojiPanel_   = new EmojiPanel(this);
    kaomojiPanel_ = new KaomojiPanel(this);
    symbolPanel_  = new SymbolPanel(this);

    pages_->addWidget(emojiPanel_);    // index 1 — TabEmoji
    pages_->addWidget(kaomojiPanel_);  // index 2 — TabKaomoji
    pages_->addWidget(symbolPanel_);   // index 3 — TabSymbol

    root->addWidget(pages_, 1);

    // ── Separator line ───────────────────────────────────────────────────
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("tabSeparator");
    root->addWidget(sep);

    // ── Tab bar ──────────────────────────────────────────────────────────
    auto* tabBar = new QWidget(this);
    tabBar->setObjectName("tabBar");
    tabBar->setFixedHeight(kTabBarHeight);
    buildTabBar(tabBar);
    root->addWidget(tabBar);

    // ── History selection ─────────────────────────────────────────────────
    auto handleSelection = [this](const QModelIndex& idx) {
        if (!idx.isValid()) return;
        auto item = idx.data(Qt::UserRole).value<core::ClipboardItem>();
        emit selected(item);
        hide();
    };
    connect(list_, &QListView::clicked,   this, handleSelection);
    connect(list_, &QListView::activated, this, handleSelection);

    // ── Picker selections ─────────────────────────────────────────────────
    auto forwardCharacter = [this](const QString& ch) {
        emit characterPasted(ch);
        hide();
    };
    connect(emojiPanel_,   &EmojiPanel::characterSelected,   this, forwardCharacter);
    connect(kaomojiPanel_, &KaomojiPanel::characterSelected, this, forwardCharacter);
    connect(symbolPanel_,  &SymbolPanel::characterSelected,  this, forwardCharacter);
}

// ─────────────────────────────────────────────────────────────────────────────

void HistoryPopup::buildTabBar(QWidget* bar)
{
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(0);

    for (int i = 0; i < 4; ++i)
    {
        auto* btn = new QPushButton(QString::fromUtf8(kTabs[i].icon), bar);
        btn->setObjectName("tabBtn");
        btn->setToolTip(kTabs[i].tip);
        btn->setFlat(true);
        btn->setCheckable(false);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setProperty("active", i == activeTab_);

        // Use a larger font for the emoji/kaomoji icons.
        QFont f = btn->font();
        f.setPointSize(i == TabKaomoji ? 12 : 18);
        btn->setFont(f);

        connect(btn, &QPushButton::clicked, this, [this, i] { switchTab(i); });

        tabButtons_.append(btn);
        layout->addWidget(btn);
    }
}

void HistoryPopup::switchTab(int index)
{
    if (index == activeTab_) return;

    // Update active property on buttons to trigger stylesheet repaint.
    tabButtons_[activeTab_]->setProperty("active", false);
    tabButtons_[activeTab_]->style()->unpolish(tabButtons_[activeTab_]);
    tabButtons_[activeTab_]->style()->polish(tabButtons_[activeTab_]);

    activeTab_ = index;
    pages_->setCurrentIndex(index);

    tabButtons_[activeTab_]->setProperty("active", true);
    tabButtons_[activeTab_]->style()->unpolish(tabButtons_[activeTab_]);
    tabButtons_[activeTab_]->style()->polish(tabButtons_[activeTab_]);

    // Let picker panels focus their search box when shown.
    switch (index)
    {
    case TabEmoji:    emojiPanel_->onShown();    break;
    case TabKaomoji:  kaomojiPanel_->onShown();  break;
    case TabSymbol:   symbolPanel_->onShown();   break;
    default: list_->setFocus(); break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void HistoryPopup::showAtTopLeft()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect screenRect = screen->availableGeometry();
        move(screenRect.topLeft() + QPoint(20, 20));
    }
    show();
    raise();
    activateWindow();

    // Always open on the history tab.
    switchTab(TabHistory);
    list_->setFocus();
}

void HistoryPopup::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }
    QWidget::keyPressEvent(event);
}

// ─────────────────────────────────────────────────────────────────────────────

void HistoryPopup::applyStyleSheet()
{
    setStyleSheet(R"(
        /* ── Popup container ── */
        #HistoryPopup {
            background: #1e1e2e;
            border: 1px solid #45475a;
            border-radius: 10px;
        }

        /* ── List views (history + picker grids) ── */
        QListView, QListWidget {
            background: transparent;
            border: none;
            color: #cdd6f4;
            outline: none;
        }

        QListView::item, QListWidget::item {
            border-radius: 5px;
            padding: 5px 8px;
        }

        QListView::item:hover, QListWidget::item:hover {
            background: rgba(137, 180, 250, 0.12);
        }

        QListView::item:selected, QListWidget::item:selected {
            background: rgba(137, 180, 250, 0.25);
            color: #89b4fa;
        }

        /* ── Search inputs ── */
        QLineEdit {
            background: #313244;
            border: 1px solid #45475a;
            border-radius: 6px;
            color: #cdd6f4;
            padding: 5px 10px;
            font-size: 13px;
            selection-background-color: #89b4fa;
        }

        QLineEdit:focus {
            border-color: #89b4fa;
        }

        /* ── Scrollbars ── */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background: #45475a;
            border-radius: 3px;
            min-height: 20px;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }

        /* ── Tab separator ── */
        #tabSeparator {
            color: #313244;
            margin: 0;
        }

        /* ── Tab bar ── */
        #tabBar {
            background: #181825;
        }

        /* ── Tab buttons ── */
        QPushButton#tabBtn {
            background: transparent;
            border: none;
            border-bottom: 2px solid transparent;
            color: #6c7086;
            padding: 4px 6px;
            border-radius: 0px;
        }

        QPushButton#tabBtn:hover {
            background: rgba(137, 180, 250, 0.08);
            color: #cdd6f4;
        }

        QPushButton#tabBtn[active="true"] {
            color: #cdd6f4;
            border-bottom: 2px solid #89b4fa;
        }
    )");
}

} // namespace ui
