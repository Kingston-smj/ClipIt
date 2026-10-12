#include "clipboard_history.h"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>
#include <algorithm>

namespace core {
namespace {
constexpr qint64 kMaxManifestBytes = 1024 * 1024;
constexpr qint64 kMaxThumbnailBytes = 256 * 1024;

bool validId(const QString& id)
{
    return !id.isEmpty() && QUuid(id).toString(QUuid::WithoutBraces) == id;
}

qsizetype textBytes(const QString& text)
{
    return text.size() * qsizetype(sizeof(QChar));
}
}

ClipboardHistory::ClipboardHistory(QString storage_dir)
    : storage_dir_(std::move(storage_dir))
{
    loadPinned();
}

void ClipboardHistory::push(const QString& text)
{
    if (!text.isEmpty() && textBytes(text) <= MemoryPolicy::MAX_TEXT_BYTES)
        pushItem(ClipboardItem::fromText(text));
}

void ClipboardHistory::push(const QImage& image)
{
    if (image.isNull())
        return;
    auto item = ClipboardItem::fromImage(image);
    if (item.byte_size <= MemoryPolicy::MAX_IMAGE_BYTES)
        pushItem(std::move(item));
}

void ClipboardHistory::pushPng(const QByteArray& png)
{
    if (png.isEmpty() || png.size() > MemoryPolicy::MAX_IMAGE_BYTES)
        return;
    auto item = ClipboardItem::fromPng(png);
    if (!item.png_data.isEmpty())
        pushItem(std::move(item));
}

void ClipboardHistory::pushItem(ClipboardItem item)
{
    if (item.isText() && std::any_of(pinned_.begin(), pinned_.end(),
        [&item](const ClipboardItem& pin) {
            return pin.isText() && pin.text == item.text;
        }))
        return;
    auto it = std::find(data_.begin(), data_.end(), item);
    if (it != data_.end())
    {
        item.id = it->id;
        item.note = it->note;
        total_bytes_ -= it->byte_size;
        data_.erase(it);
    }
    evictToFit(item.byte_size);
    total_bytes_ += item.byte_size;
    data_.push_front(std::move(item));
}

void ClipboardHistory::evictToFit(qsizetype incoming_bytes)
{
    while (!data_.empty() &&
           (data_.size() >= MemoryPolicy::MAX_ITEMS ||
            total_bytes_ + incoming_bytes > MemoryPolicy::MAX_TOTAL_BYTES))
    {
        total_bytes_ -= data_.back().byte_size;
        data_.pop_back();
    }
}

const std::deque<ClipboardItem>& ClipboardHistory::items() const
{
    return data_;
}

const ClipboardItem* ClipboardHistory::find(const QString& id) const
{
    for (const auto& item : pinned_)
        if (item.id == id)
            return &item;
    for (const auto& item : data_)
        if (item.id == id)
            return &item;
    return nullptr;
}

ClipboardItem ClipboardHistory::itemForPaste(const QString& id) const
{
    const ClipboardItem* item = find(id);
    if (!item)
        return {};
    ClipboardItem result = *item;
    if (result.isImage() && result.png_data.isEmpty())
        result.png_data = loadImage(result);
    return result;
}

bool ClipboardHistory::pin(const QString& id)
{
    last_error_.clear();
    auto it = std::find_if(data_.begin(), data_.end(),
        [&id](const ClipboardItem& item) { return item.id == id; });
    if (it == data_.end())
        return false;
    if (pinned_.size() >= MemoryPolicy::MAX_PINNED_ITEMS)
    {
        setError(QStringLiteral("The pinned-item limit has been reached."));
        return false;
    }
    qsizetype bytes = it->byte_size;
    for (const auto& item : pinned_)
        bytes += item.byte_size;
    if (bytes > MemoryPolicy::MAX_PINNED_BYTES)
    {
        setError(QStringLiteral("The pinned storage limit has been reached."));
        return false;
    }
    ClipboardItem item = *it;
    item.pinned = true;
    bool wrote_image = false;
    if (item.isImage() && !storage_dir_.isEmpty())
    {
        if (!QDir().mkpath(storage_dir_))
        {
            setError(QStringLiteral("Cannot create the pinned-items directory."));
            return false;
        }
        QFile::setPermissions(storage_dir_, QFileDevice::ReadOwner |
            QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        QSaveFile file(imagePath(item.id));
        if (!file.open(QIODevice::WriteOnly) ||
            file.write(item.png_data) != item.png_data.size() || !file.commit())
        {
            setError(QStringLiteral("Cannot save the pinned image."));
            return false;
        }
        QFile::setPermissions(imagePath(item.id), QFileDevice::ReadOwner |
            QFileDevice::WriteOwner);
        item.png_data.clear();
        wrote_image = true;
    }
    auto candidate = pinned_;
    candidate.push_front(item);
    if (!savePinned(candidate))
    {
        if (wrote_image)
            QFile::remove(imagePath(item.id));
        return false;
    }
    pinned_.swap(candidate);
    total_bytes_ -= it->byte_size;
    data_.erase(it);
    return true;
}

bool ClipboardHistory::unpin(const QString& id)
{
    last_error_.clear();
    auto it = std::find_if(pinned_.begin(), pinned_.end(),
        [&id](const ClipboardItem& item) { return item.id == id; });
    if (it == pinned_.end())
        return false;
    ClipboardItem item = *it;
    if (item.isImage() && item.png_data.isEmpty())
    {
        item.png_data = loadImage(item);
        if (item.png_data.isEmpty())
        {
            setError(QStringLiteral("Cannot read the pinned image."));
            return false;
        }
    }
    auto candidate = pinned_;
    candidate.erase(candidate.begin() + std::distance(pinned_.begin(), it));
    if (!savePinned(candidate))
        return false;
    pinned_.swap(candidate);
    if (item.isImage() && !storage_dir_.isEmpty())
        QFile::remove(imagePath(item.id));
    item.pinned = false;
    evictToFit(item.byte_size);
    total_bytes_ += item.byte_size;
    data_.push_front(std::move(item));
    return true;
}

bool ClipboardHistory::edit(const QString& id, const QString& value)
{
    last_error_.clear();
    auto change = [this, &value](ClipboardItem& item) {
        if (item.isText())
        {
            if (value.isEmpty() || textBytes(value) > MemoryPolicy::MAX_TEXT_BYTES)
            {
                setError(QStringLiteral("Text must be 1–64 KiB."));
                return false;
            }
            item.text = value;
            item.byte_size = textBytes(value);
        }
        else
        {
            if (value.toUtf8().size() > MemoryPolicy::MAX_NOTE_BYTES)
            {
                setError(QStringLiteral("An image note can be at most 1 KiB."));
                return false;
            }
            item.note = value;
        }
        return true;
    };
    for (size_t i = 0; i < pinned_.size(); ++i)
    {
        if (pinned_[i].id != id)
            continue;
        auto candidate = pinned_;
        if (!change(candidate[i]))
            return false;
        qsizetype bytes = 0;
        for (const auto& item : candidate)
            bytes += item.byte_size;
        if (bytes > MemoryPolicy::MAX_PINNED_BYTES)
        {
            setError(QStringLiteral("The pinned storage limit has been reached."));
            return false;
        }
        if (!savePinned(candidate))
            return false;
        pinned_.swap(candidate);
        return true;
    }
    for (auto& item : data_)
    {
        if (item.id != id)
            continue;
        ClipboardItem candidate = item;
        if (!change(candidate))
            return false;
        if (total_bytes_ - item.byte_size + candidate.byte_size >
            MemoryPolicy::MAX_TOTAL_BYTES)
        {
            setError(QStringLiteral("The recent-history memory limit has been reached."));
            return false;
        }
        total_bytes_ += candidate.byte_size - item.byte_size;
        item = std::move(candidate);
        return true;
    }
    return false;
}

bool ClipboardHistory::remove(const QString& id)
{
    last_error_.clear();
    auto pin = std::find_if(pinned_.begin(), pinned_.end(),
        [&id](const ClipboardItem& item) { return item.id == id; });
    if (pin != pinned_.end())
    {
        const bool image = pin->isImage();
        auto candidate = pinned_;
        candidate.erase(candidate.begin() + std::distance(pinned_.begin(), pin));
        if (!savePinned(candidate))
            return false;
        pinned_.swap(candidate);
        if (image && !storage_dir_.isEmpty())
            QFile::remove(imagePath(id));
        return true;
    }
    auto it = std::find_if(data_.begin(), data_.end(),
        [&id](const ClipboardItem& item) { return item.id == id; });
    if (it == data_.end())
        return false;
    total_bytes_ -= it->byte_size;
    data_.erase(it);
    return true;
}

QString ClipboardHistory::imagePath(const QString& id) const
{
    return storage_dir_ + QStringLiteral("/") + id + QStringLiteral(".png");
}

QByteArray ClipboardHistory::loadImage(const ClipboardItem& item) const
{
    if (storage_dir_.isEmpty() || !validId(item.id))
        return {};
    QFile file(imagePath(item.id));
    if (!file.open(QIODevice::ReadOnly) || file.size() != item.byte_size ||
        file.size() > MemoryPolicy::MAX_IMAGE_BYTES)
        return {};
    return file.readAll();
}

void ClipboardHistory::setError(const QString& error)
{
    last_error_ = error;
}

bool ClipboardHistory::savePinned(const std::deque<ClipboardItem>& candidate)
{
    if (storage_dir_.isEmpty())
        return true;
    if (!QDir().mkpath(storage_dir_))
    {
        setError(QStringLiteral("Cannot create the pinned-items directory."));
        return false;
    }
    QFile::setPermissions(storage_dir_, QFileDevice::ReadOwner |
        QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QJsonArray entries;
    for (const auto& item : candidate)
    {
        QJsonObject entry;
        entry.insert(QStringLiteral("id"), item.id);
        entry.insert(QStringLiteral("type"), item.isText() ? QStringLiteral("text") :
            QStringLiteral("image"));
        entry.insert(QStringLiteral("bytes"), double(item.byte_size));
        if (item.isText())
            entry.insert(QStringLiteral("text"), item.text);
        else
        {
            entry.insert(QStringLiteral("note"), item.note);
            QByteArray thumbnail;
            QBuffer buffer(&thumbnail);
            if (!buffer.open(QIODevice::WriteOnly) ||
                !item.thumbnail.save(&buffer, "PNG") ||
                thumbnail.size() > kMaxThumbnailBytes)
            {
                setError(QStringLiteral("Cannot save the image thumbnail."));
                return false;
            }
            entry.insert(QStringLiteral("thumbnail"),
                         QString::fromLatin1(thumbnail.toBase64()));
        }
        entries.append(entry);
    }
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("items"), entries);
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (json.size() > kMaxManifestBytes)
    {
        setError(QStringLiteral("Pinned metadata is too large."));
        return false;
    }
    const QString path = storage_dir_ + QStringLiteral("/manifest.json");
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() ||
        !file.commit())
    {
        setError(QStringLiteral("Cannot save pinned metadata."));
        return false;
    }
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return true;
}

void ClipboardHistory::loadPinned()
{
    if (storage_dir_.isEmpty())
        return;
    QFile file(storage_dir_ + QStringLiteral("/manifest.json"));
    if (!file.open(QIODevice::ReadOnly) || file.size() > kMaxManifestBytes)
        return;
    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject())
        return;
    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("version")).toInt() != 1)
        return;
    const QJsonArray entries = root.value(QStringLiteral("items")).toArray();
    qsizetype total = 0;
    for (const QJsonValue& value : entries)
    {
        if (pinned_.size() >= MemoryPolicy::MAX_PINNED_ITEMS)
            break;
        const QJsonObject entry = value.toObject();
        ClipboardItem item;
        item.id = entry.value(QStringLiteral("id")).toString();
        if (!validId(item.id) || find(item.id))
            continue;
        item.pinned = true;
        const QString type = entry.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("text"))
        {
            item = ClipboardItem::fromText(entry.value(QStringLiteral("text")).toString());
            item.id = entry.value(QStringLiteral("id")).toString();
            item.pinned = true;
            if (item.text.isEmpty() || item.byte_size > MemoryPolicy::MAX_TEXT_BYTES)
                continue;
        }
        else if (type == QStringLiteral("image"))
        {
            item.type = ClipboardType::Image;
            item.note = entry.value(QStringLiteral("note")).toString();
            if (item.note.toUtf8().size() > MemoryPolicy::MAX_NOTE_BYTES)
                continue;
            const qint64 bytes = qint64(entry.value(QStringLiteral("bytes")).toDouble());
            const QFileInfo info(imagePath(item.id));
            if (bytes <= 0 || bytes > MemoryPolicy::MAX_IMAGE_BYTES ||
                !info.isFile() || info.size() != bytes)
                continue;
            const QByteArray encoded = entry.value(QStringLiteral("thumbnail"))
                .toString().toLatin1();
            if (encoded.size() > kMaxThumbnailBytes * 4 / 3 + 4)
                continue;
            QByteArray thumbnail = QByteArray::fromBase64(encoded);
            if (thumbnail.isEmpty() || thumbnail.size() > kMaxThumbnailBytes)
                continue;
            QBuffer buffer(&thumbnail);
            if (!buffer.open(QIODevice::ReadOnly))
                continue;
            QImageReader reader(&buffer, "PNG");
            const QSize size = reader.size();
            if (!size.isValid() || size.width() > 128 || size.height() > 128)
                continue;
            item.thumbnail = reader.read();
            if (item.thumbnail.isNull())
                continue;
            item.byte_size = bytes;
        }
        else
            continue;
        if (total + item.byte_size > MemoryPolicy::MAX_PINNED_BYTES)
            continue;
        total += item.byte_size;
        pinned_.push_back(std::move(item));
    }
}

} // namespace core
