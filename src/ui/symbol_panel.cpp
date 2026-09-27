#include "symbol_panel.h"
#include "data/symbol_data.h"

#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFont>

namespace ui {

SymbolPanel::SymbolPanel(QWidget* parent)
    : PickerPanel(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 4);
    layout->setSpacing(6);

    // ── Search box ───────────────────────────────────────────────────────
    search_ = new QLineEdit(this);
    search_->setPlaceholderText("Search symbols…");
    search_->setClearButtonEnabled(true);
    layout->addWidget(search_);

    // ── Symbol grid ──────────────────────────────────────────────────────
    grid_ = new QListWidget(this);
    grid_->setViewMode(QListView::IconMode);
    grid_->setUniformItemSizes(true);
    grid_->setGridSize(QSize(46, 46));
    grid_->setIconSize(QSize(1, 1));
    grid_->setResizeMode(QListView::Adjust);
    grid_->setMovement(QListView::Static);
    grid_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid_->setSelectionMode(QAbstractItemView::SingleSelection);
    grid_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_->setWordWrap(false);
    grid_->setSpacing(2);

    // Symbol font — slightly smaller than emoji so glyphs fit the cell.
    QFont symFont;
    symFont.setPointSize(16);
    grid_->setFont(symFont);

    layout->addWidget(grid_, 1);

    // Population is deferred to onShown() so startup is instant.

    // ── Connections ──────────────────────────────────────────────────────
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& text) {
        populate(text.trimmed());
    });

    auto emitSelected = [this](QListWidgetItem* item) {
        if (!item) return;
        emit characterSelected(item->data(Qt::UserRole).toString());
    };

    connect(grid_, &QListWidget::itemActivated,     this, emitSelected);
    connect(grid_, &QListWidget::itemDoubleClicked, this, emitSelected);
}

void SymbolPanel::onShown()
{
    if (!populated_)
    {
        populate({});
        populated_ = true;
    }
    search_->setFocus();
    search_->selectAll();
}

void SymbolPanel::populate(const QString& filter)
{
    const QString lower = filter.toLower();

    grid_->setUpdatesEnabled(false);
    grid_->clear();

    for (std::size_t i = 0; i < data::kSymbolCount; ++i)
    {
        const data::SymbolEntry& e = data::kSymbols[i];

        if (!lower.isEmpty())
        {
            const QString label    = QString::fromUtf8(e.label);
            const QString category = QString::fromUtf8(e.category);
            const QString glyph    = QString::fromUtf8(e.glyph);
            if (!label.contains(lower) && !category.contains(lower)
                    && !glyph.contains(lower))
                continue;
        }

        auto* item = new QListWidgetItem(QString::fromUtf8(e.glyph));
        item->setData(Qt::UserRole, QString::fromUtf8(e.glyph));
        item->setToolTip(QString("%1 (%2)")
            .arg(QString::fromUtf8(e.label),
                 QString::fromUtf8(e.category)));
        item->setTextAlignment(Qt::AlignCenter);
        grid_->addItem(item);
    }

    grid_->setUpdatesEnabled(true);
}

} // namespace ui
