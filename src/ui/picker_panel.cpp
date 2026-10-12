#include "picker_panel.h"
#include "picker_diagnostics.h"
#include "app/logging.h"
#include <cstdio>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

// This translation unit exists solely to give Qt's MOC a place to emit
// the meta-object machinery for PickerPanel (vtable, staticMetaObject,
// characterSelected signal body, etc.).  Without it the linker cannot
// resolve references from the subclass moc_*.cpp files.

namespace ui {
namespace {
qint64 residentKiB()
{
#ifdef Q_OS_LINUX
    if (std::FILE* file = std::fopen("/proc/self/statm", "r"))
    {
        unsigned long total = 0, resident = 0;
        const int fields = std::fscanf(file, "%lu %lu", &total, &resident);
        std::fclose(file);
        const long page_size = ::sysconf(_SC_PAGESIZE);
        if (fields == 2 && page_size > 0)
            return static_cast<qint64>(resident) * page_size / 1024;
    }
#endif
    return -1;
}
}

DiagnosticPickerList::DiagnosticPickerList(const char* name, QWidget* parent)
    : QListWidget(parent), name_(name),
      enabled_(qEnvironmentVariableIntValue("CLIPIT_PICKER_DIAGNOSTICS") == 1)
{}

void DiagnosticPickerList::beginSample()
{
    if (!enabled_ || awaiting_paint_)
        return;
    ++sample_;
    rss_before_ = residentKiB();
    awaiting_paint_ = true;
    sample_timer_.start();
}

void DiagnosticPickerList::beginPopulation()
{
    if (!enabled_)
        return;
    beginSample();
    population_timer_.start();
}

void DiagnosticPickerList::endPopulation()
{
    if (!enabled_)
        return;
    const double ms = population_timer_.nsecsElapsed() / 1000000.0;
    app::log_info("PickerDiagnostic panel=%s sample=%u phase=population ms=%.3f items=%d rss_kib=%lld",
                  name_, sample_, ms, count(), static_cast<long long>(residentKiB()));
}

void DiagnosticPickerList::doItemsLayout()
{
    if (!enabled_)
    {
        QListWidget::doItemsLayout();
        return;
    }
    QElapsedTimer timer;
    timer.start();
    QListWidget::doItemsLayout();
    const double ms = timer.nsecsElapsed() / 1000000.0;
    app::log_info("PickerDiagnostic panel=%s sample=%u phase=layout ms=%.3f items=%d",
                  name_, sample_, ms, count());
}

void DiagnosticPickerList::showEvent(QShowEvent* event)
{
    beginSample();
    QListWidget::showEvent(event);
}

void DiagnosticPickerList::paintEvent(QPaintEvent* event)
{
    if (!awaiting_paint_)
    {
        QListWidget::paintEvent(event);
        return;
    }
    QElapsedTimer timer;
    timer.start();
    QListWidget::paintEvent(event);
    const double paint_ms = timer.nsecsElapsed() / 1000000.0;
    const double elapsed_ms = sample_timer_.nsecsElapsed() / 1000000.0;
    awaiting_paint_ = false;
    app::log_info("PickerDiagnostic panel=%s sample=%u phase=first-paint paint_ms=%.3f elapsed_ms=%.3f rss_before_kib=%lld rss_after_kib=%lld",
                  name_, sample_, paint_ms, elapsed_ms,
                  static_cast<long long>(rss_before_), static_cast<long long>(residentKiB()));
}

} // namespace ui
