#pragma once

#include <QString>
#include <QByteArray>
#include <QImage>
#include <QMetaType>

namespace core {

enum class ClipboardType { Text, Image };

struct ClipboardItem
{
    ClipboardType type      = ClipboardType::Text;
    QString       text;        // valid when type == Text
    QByteArray    png_data;    // PNG-compressed bytes, valid when type == Image
    QImage        thumbnail;   // 128×128 preview,     valid when type == Image
    qsizetype     byte_size{0};// memory footprint of this item

    bool isText()  const { return type == ClipboardType::Text;  }
    bool isImage() const { return type == ClipboardType::Image; }

    // Factory helpers — defined in clipboard_item.cpp
    static ClipboardItem fromText(const QString& text);
    static ClipboardItem fromImage(const QImage& image);

    bool operator==(const ClipboardItem& other) const;
};

} // namespace core

Q_DECLARE_METATYPE(core::ClipboardItem)
