#pragma once

#include "picker_panel.h"
#include "data/emoji_data.h"

class QListWidget;
class QLineEdit;

namespace ui {

// Grid picker backed by the compile-time emoji table.
// Displays a searchable, scrollable grid of Unicode emoji characters.
class EmojiPanel : public PickerPanel
{
    Q_OBJECT

public:
    explicit EmojiPanel(QWidget* parent = nullptr);

    void onShown() override;

private:
    void populate(const QString& filter);

    QLineEdit*    search_;
    QListWidget*  grid_;
    bool          populated_{false};   // lazy: don't load until first shown
};

} // namespace ui
