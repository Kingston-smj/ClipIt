#pragma once

#include "picker_panel.h"

class QListWidget;
class QLineEdit;

namespace ui {

// Grid picker backed by the compile-time symbol table.
// Displays a searchable, scrollable grid of Unicode symbols.
class SymbolPanel : public PickerPanel
{
    Q_OBJECT

public:
    explicit SymbolPanel(QWidget* parent = nullptr);

    void onShown() override;

private:
    void populate(const QString& filter);

    QLineEdit*    search_;
    QListWidget*  grid_;
    bool          populated_{false};
};

} // namespace ui
