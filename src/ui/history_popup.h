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

    void showPopup();

signals:
    void selected(const QString& id);

    // Emitted when the user picks an emoji, kaomoji, or symbol.
    // The controller places the character on the clipboard directly.
    void characterPasted(const QString& ch);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void switchTab(int index);
    void buildTabBar(QWidget* bar);
    void applyStyleSheet();
    void showItemMenu(HistoryModel& model, const QPoint& pos);
    void editItem(HistoryModel& model, const QString& id);

    // Pages (order must match tab indices below).
    enum Tab { TabHistory = 0, TabEmoji = 1, TabKaomoji = 2, TabSymbol = 3 };

    QStackedWidget* pages_;
    QWidget*        dragHeader_;
    QListView*      list_;           // page 0 — clipboard history
    EmojiPanel*     emojiPanel_;     // page 1
    KaomojiPanel*   kaomojiPanel_;   // page 2
    SymbolPanel*     symbolPanel_;    // page 3

    QList<QPushButton*> tabButtons_;
    int activeTab_{ TabHistory };
};

} // namespace ui
