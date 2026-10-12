#pragma once

#include "picker_panel.h"

class QLineEdit;

namespace ui {

class DiagnosticPickerList;

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
    DiagnosticPickerList*  grid_;
    bool          populated_{false};
};

} // namespace ui
