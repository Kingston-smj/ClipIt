#pragma once

#include "picker_panel.h"

class QListWidget;
class QLineEdit;

namespace ui {

// List picker backed by the compile-time kaomoji table.
// Displays searchable text-art emoticons in a vertical list.
class KaomojiPanel : public PickerPanel
{
    Q_OBJECT

public:
    explicit KaomojiPanel(QWidget* parent = nullptr);

    void onShown() override;

private:
    void populate(const QString& filter);

    QLineEdit*    search_;
    QListWidget*  grid_;
    bool          populated_{false};
};

} // namespace ui
