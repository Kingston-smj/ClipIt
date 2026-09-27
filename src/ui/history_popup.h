#pragma once

#include <QWidget>
#include <QString>
#include "core/clipboard_item.h"

class QListView;
class QKeyEvent;
class QStackedWidget;
class QPushButton;

namespace ui {

class HistoryModel;
class EmojiPanel;
class KaomojiPanel;
class SymbolPanel;

class HistoryPopup : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPopup(HistoryModel& model, QWidget* parent = nullptr);

    void showAtTopLeft();

signals:
    // Emits the full ClipboardItem so the controller can restore
    // both text and images to the system clipboard correctly.
    void selected(const core::ClipboardItem& item);

    // Emitted when the user picks an emoji, kaomoji, or symbol.
    // The controller places the character on the clipboard directly.
    void characterPasted(const QString& ch);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void switchTab(int index);
    void buildTabBar(QWidget* bar);
    void applyStyleSheet();

    // Pages (order must match tab indices below).
    enum Tab { TabHistory = 0, TabEmoji = 1, TabKaomoji = 2, TabSymbol = 3 };

    QStackedWidget* pages_;
    QListView*      list_;           // page 0 — clipboard history
    EmojiPanel*     emojiPanel_;     // page 1
    KaomojiPanel*   kaomojiPanel_;   // page 2
    SymbolPanel*     symbolPanel_;    // page 3

    QList<QPushButton*> tabButtons_;
    int activeTab_{ TabHistory };
};

} // namespace ui
