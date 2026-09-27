#include "emoji_panel.h"
#include "data/emoji_data.h"

#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFont>

namespace ui {

EmojiPanel::EmojiPanel(QWidget* parent)
    : PickerPanel(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 4);
    layout->setSpacing(6);

    // ── Search box ───────────────────────────────────────────────────────
    search_ = new QLineEdit(this);
    search_->setPlaceholderText("Search emoji…");
    search_->setClearButtonEnabled(true);
    layout->addWidget(search_);

    // ── Emoji grid ───────────────────────────────────────────────────────
    grid_ = new QListWidget(this);
    grid_->setViewMode(QListView::IconMode);
    grid_->setUniformItemSizes(true);
    grid_->setGridSize(QSize(46, 46));
    grid_->setIconSize(QSize(1, 1));    // text-only; no real icons
    grid_->setResizeMode(QListView::Adjust);
    grid_->setMovement(QListView::Static);
    grid_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid_->setSelectionMode(QAbstractItemView::SingleSelection);
    grid_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_->setWordWrap(false);
    grid_->setSpacing(2);

    // Use a system emoji font at a comfortable size.
    QFont emojiFont;
    emojiFont.setPointSize(22);
    grid_->setFont(emojiFont);

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

    connect(grid_, &QListWidget::itemActivated,  this, emitSelected);
    connect(grid_, &QListWidget::itemDoubleClicked, this, emitSelected);
}

void EmojiPanel::onShown()
{
    if (!populated_)
    {
        populate({});
        populated_ = true;
    }
    search_->setFocus();
    search_->selectAll();
}

void EmojiPanel::populate(const QString& filter)
{
    const QString lower = filter.toLower();

    grid_->setUpdatesEnabled(false);   // batch: suppress per-item layout
    grid_->clear();

    for (std::size_t i = 0; i < data::kEmojiCount; ++i)
    {
        const data::EmojiEntry& e = data::kEmoji[i];

        if (!lower.isEmpty())
        {
            // Match against label or category (case-insensitive).
            const QString label    = QString::fromUtf8(e.label);
            const QString category = QString::fromUtf8(e.category);
            if (!label.contains(lower) && !category.contains(lower))
                continue;
        }

        auto* item = new QListWidgetItem(QString::fromUtf8(e.glyph));
        item->setData(Qt::UserRole, QString::fromUtf8(e.glyph));
        item->setToolTip(QString::fromUtf8(e.label));
        item->setTextAlignment(Qt::AlignCenter);
        grid_->addItem(item);
    }

    grid_->setUpdatesEnabled(true);
}

} // namespace ui
