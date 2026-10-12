#include "clipboard_item.h"
#include <QBuffer>
#include <QImageReader>

namespace core {

// ── Text ─────────────────────────────────────────────────────────────────────

ClipboardItem ClipboardItem::fromText(const QString& text)
{
    ClipboardItem item;
    item.type      = ClipboardType::Text;
    item.text      = text;
    item.byte_size = static_cast<qsizetype>(text.size()) * sizeof(QChar);
    return item;
}

// ── Image ─────────────────────────────────────────────────────────────────────

ClipboardItem ClipboardItem::fromImage(const QImage& image)
{
    ClipboardItem item;
    item.type = ClipboardType::Image;

    // Compress to PNG — much smaller than raw pixels for storage.
    QBuffer buf(&item.png_data);
    buf.open(QIODevice::WriteOnly);
    image.save(&buf, "PNG");
    buf.close();

    // Pre-scale thumbnail; always kept regardless of byte budget.
    item.thumbnail = image.scaled(128, 128,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);

    item.byte_size = item.png_data.size();
    return item;
}

ClipboardItem ClipboardItem::fromPng(QByteArray png)
{
    ClipboardItem item;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::ReadOnly))
        return item;
    QImageReader reader(&buffer, "PNG");
    const QSize size = reader.size();
    if (!size.isValid())
        return item;
    reader.setScaledSize(size.scaled(128, 128, Qt::KeepAspectRatio));
    QImage thumbnail = reader.read();
    if (thumbnail.isNull())
        return item;
    item.type = ClipboardType::Image;
    item.byte_size = png.size();
    item.png_data = std::move(png);
    item.thumbnail = std::move(thumbnail);
    return item;
}

// ── Equality ──────────────────────────────────────────────────────────────────

bool ClipboardItem::operator==(const ClipboardItem& other) const
{
    if (type != other.type)
        return false;
    if (isText())
        return text == other.text;
    // For images: compare compressed bytes (exact equality).
    return png_data == other.png_data;
}

} // namespace core
