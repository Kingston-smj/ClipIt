#pragma once

#include <QElapsedTimer>
#include <QListWidget>

namespace ui {

// BUG-003 instrumentation, enabled with CLIPIT_PICKER_DIAGNOSTICS=1.
// No polling or stored sample history.
class DiagnosticPickerList : public QListWidget
{
public:
    explicit DiagnosticPickerList(const char* name, QWidget* parent);
    void beginPopulation();
    void endPopulation();
    void doItemsLayout() override;

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void beginSample();
    const char* name_;
    const bool enabled_;
    QElapsedTimer sample_timer_;
    QElapsedTimer population_timer_;
    qint64 rss_before_{-1};
    unsigned sample_{0};
    bool awaiting_paint_{false};
};

} // namespace ui
