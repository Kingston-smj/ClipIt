#include "emoji_panel.h"
#include "picker_diagnostics.h"
#include "data/emoji_data.h"

#include <QLineEdit>
#include <QTabBar>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFont>
#include <QApplication>
#include <QPainter>
#include <QStyledItemDelegate>
#include <cstring>

namespace ui {
namespace {

struct CategoryIcon { const char* category; const char* icon; };
constexpr CategoryIcon kCategoryIcons[] = {
    {"Smileys", "😀"}, {"People", "👥"}, {"Animals", "🐾"},
    {"Nature", "🌿"}, {"Food", "🍔"}, {"Travel", "🚗"},
    {"Activities", "⚽"}, {"Objects", "💡"}, {"Symbols", "❤️"},
};

const char* iconFor(const char* category)
{
    for (const auto& entry : kCategoryIcons)
        if (std::strcmp(entry.category, category) == 0)
            return entry.icon;
    return category;
}

class EmojiDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override
    {
        return QSize(46, 46);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem background(option);
        initStyleOption(&background, index);
        const QString glyph = background.text;
        background.text.clear();
        background.features &= ~QStyleOptionViewItem::HasDisplay;
        background.showDecorationSelected = true;
        const QStyle* style = option.widget ? option.widget->style() : QApplication::style();

        painter->save();
        painter->setClipRect(option.rect, Qt::IntersectClip);
        style->drawControl(QStyle::CE_ItemViewItem, &background, painter, option.widget);
        painter->setFont(background.font);
        painter->setPen(background.palette.color(
            option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
        painter->drawText(option.rect.adjusted(2, 2, -2, -2),
                          Qt::AlignCenter | Qt::TextSingleLine, glyph);
        painter->restore();
    }
};

} // namespace

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

    categories_ = new QTabBar(this);
    categories_->setObjectName("pickerCategories");
    categories_->setExpanding(false);
    categories_->setUsesScrollButtons(true);
    const char* previous = nullptr;
    for (const auto& entry : data::kEmoji)
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

    // ── Emoji grid ───────────────────────────────────────────────────────
    grid_ = new DiagnosticPickerList("emoji", this);
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
    grid_->setItemDelegate(new EmojiDelegate(grid_));

    // Use a system emoji font at a comfortable size.
    QFont emojiFont;
    // Use the installed color font; Qt still falls back if it is unavailable.
    emojiFont.setFamilies({QStringLiteral("Noto Color Emoji"), QStringLiteral("emoji")});
    emojiFont.setPointSize(22);
    grid_->setFont(emojiFont);

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
    grid_->beginPopulation();
    const QString lower = filter.toLower();
    const QString selected = categories_->tabData(categories_->currentIndex()).toString();
    heading_->setText(lower.isEmpty() ? selected : tr("Search results"));

    grid_->setUpdatesEnabled(false);   // batch: suppress per-item layout
    grid_->clear();

    for (std::size_t i = 0; i < data::kEmojiCount; ++i)
    {
        const data::EmojiEntry& e = data::kEmoji[i];

        if (lower.isEmpty() && QString::fromUtf8(e.category) != selected)
            continue;

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
    grid_->endPopulation();
}

} // namespace ui
