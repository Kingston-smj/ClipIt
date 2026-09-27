#pragma once

#include <QWidget>
#include <QString>

namespace ui {

// Abstract base for all picker panels (emoji / kaomoji / symbol).
// Each panel emits characterSelected() when the user picks an item.
class PickerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PickerPanel(QWidget* parent = nullptr) : QWidget(parent) {}

    // Called by HistoryPopup whenever this panel becomes visible,
    // so the panel can focus its search box.
    virtual void onShown() {}

signals:
    void characterSelected(const QString& ch);
};

} // namespace ui
