#include "kaomoji_panel.h"
#include "data/kaomoji_data.h"

#include <QLineEdit>
#include <QTabBar>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFont>
#include <cstring>

namespace ui {
namespace {

struct CategoryIcon { const char* category; const char* icon; };
constexpr CategoryIcon kCategoryIcons[] = {
    {"Happy", "(^_^)"}, {"Sad", "(T_T)"}, {"Angry", "(ಠ_ಠ)"},
    {"Surprised", "(°o°)"}, {"Love", "(♡_♡)"}, {"Cute", "=^.^="},
    {"Tired", "(-_-)"}, {"Misc", "(ツ)"},
};

const char* iconFor(const char* category)
{
    for (const auto& entry : kCategoryIcons)
        if (std::strcmp(entry.category, category) == 0)
            return entry.icon;
    return category;
}

} // namespace

KaomojiPanel::KaomojiPanel(QWidget* parent)
    : PickerPanel(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 4);
    layout->setSpacing(6);

    // ── Search box ───────────────────────────────────────────────────────
    search_ = new QLineEdit(this);
    search_->setPlaceholderText("Search kaomoji…");
    search_->setClearButtonEnabled(true);
    layout->addWidget(search_);

    categories_ = new QTabBar(this);
    categories_->setObjectName("pickerCategories");
    categories_->setProperty("kind", "kaomoji");
    categories_->setExpanding(false);
    categories_->setUsesScrollButtons(true);
    const char* previous = nullptr;
    for (const auto& entry : data::kKaomoji)
    {
        if (!previous || std::strcmp(previous, entry.category) != 0)
        {
            const QString name = QString::fromUtf8(entry.category);
            const int tab = categories_->addTab(QString::fromUtf8(iconFor(entry.category)));
            categories_->setTabData(tab, name);
            categories_->setTabToolTip(tab, name);
            categories_->setAccessibleTabName(tab, name);
        }
        previous = entry.category;
    }
    layout->addWidget(categories_);

    heading_ = new QLabel(this);
    heading_->setObjectName("pickerHeading");
    layout->addWidget(heading_);

    // ── Kaomoji grid ─────────────────────────────────────────────────────
    // 108 px cells → 3 per row in the 360 px popup.
    // Monospace keeps ASCII art alignment clean.
    grid_ = new QListWidget(this);
    grid_->setViewMode(QListView::IconMode);
    grid_->setUniformItemSizes(true);
    grid_->setGridSize(QSize(108, 42));
    grid_->setIconSize(QSize(1, 1));        // text-only; no real icons
    grid_->setResizeMode(QListView::Adjust);
    grid_->setMovement(QListView::Static);
    grid_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid_->setSelectionMode(QAbstractItemView::SingleSelection);
    grid_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_->setWordWrap(false);
    grid_->setSpacing(2);

    // Monospace so ASCII art aligns correctly.
    QFont mono;
    mono.setFamily("monospace");
    mono.setPointSize(10);
    grid_->setFont(mono);

    layout->addWidget(grid_, 1);

    // Population is deferred to onShown() so startup is instant.

    // ── Connections ──────────────────────────────────────────────────────
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& text) {
        populate(text.trimmed());
    });
    connect(categories_, &QTabBar::currentChanged, this, [this](int) {
        if (search_->text().isEmpty())
            populate({});
        else
            search_->clear();
    });

    auto emitSelected = [this](QListWidgetItem* item) {
        if (!item) return;
        emit characterSelected(item->data(Qt::UserRole).toString());
    };

    connect(grid_, &QListWidget::itemClicked,   this, emitSelected);
    connect(grid_, &QListWidget::itemActivated, this, emitSelected);
}

void KaomojiPanel::onShown()
{
    if (!populated_)
    {
        populate({});
        populated_ = true;
    }
    search_->setFocus();
    search_->selectAll();
}

void KaomojiPanel::populate(const QString& filter)
{
    const QString lower = filter.toLower();
    const QString selected = categories_->tabData(categories_->currentIndex()).toString();
    heading_->setText(lower.isEmpty() ? selected : tr("Search results"));

    grid_->setUpdatesEnabled(false);
    grid_->clear();

    for (std::size_t i = 0; i < data::kKaomojiCount; ++i)
    {
        const data::KaomojiEntry& e = data::kKaomoji[i];

        if (lower.isEmpty() && QString::fromUtf8(e.category) != selected)
            continue;

        if (!lower.isEmpty())
        {
            const QString label    = QString::fromUtf8(e.label);
            const QString category = QString::fromUtf8(e.category);
            const QString text     = QString::fromUtf8(e.text);
            if (!label.contains(lower) && !category.contains(lower)
                    && !text.contains(lower))
                continue;
        }

        auto* item = new QListWidgetItem(QString::fromUtf8(e.text));
        item->setData(Qt::UserRole, QString::fromUtf8(e.text));
        // Show label + category in tooltip since they're no longer visible.
        item->setToolTip(QString("%1  ·  %2")
            .arg(QString::fromUtf8(e.label),
                 QString::fromUtf8(e.category)));
        item->setTextAlignment(Qt::AlignCenter);
        grid_->addItem(item);
    }

    grid_->setUpdatesEnabled(true);
}

} // namespace ui
