#include <QTest>
#include <QImage>
#include <QBuffer>
#include <QTemporaryDir>
#include "core/clipboard_history.h"
#include "core/clipboard_item.h"
#include "core/memory_policy.h"

class TestHistory : public QObject
{
    Q_OBJECT

private slots:
    void testPushSingle()
    {
        core::ClipboardHistory history;
        history.push("hello");
        QCOMPARE(history.items().size(), static_cast<size_t>(1));
        QCOMPARE(history.items().front().text, QString("hello"));
        QVERIFY(history.items().front().isText());
    }

    void testPushEmpty()
    {
        core::ClipboardHistory history;
        history.push("");
        QCOMPARE(history.items().size(), static_cast<size_t>(0));
    }

    void testMruPromotion()
    {
        core::ClipboardHistory history;
        history.push("alpha");
        history.push("beta");
        history.push("gamma");

        // Re-push "alpha" — it should move to front.
        history.push("alpha");

        QCOMPARE(history.items().size(), static_cast<size_t>(3));
        QCOMPARE(history.items()[0].text, QString("alpha"));
        QCOMPARE(history.items()[1].text, QString("gamma"));
        QCOMPARE(history.items()[2].text, QString("beta"));
    }

    void testMaxCapacity()
    {
        core::ClipboardHistory history;
        for (int i = 0; i < 15; ++i)
            history.push(QString("item_%1").arg(i));

        QCOMPARE(history.items().size(), static_cast<size_t>(10));
        QCOMPARE(history.items().front().text, QString("item_14"));
        QCOMPARE(history.items().back().text,  QString("item_5"));
    }

    void testImagePush()
    {
        core::ClipboardHistory history;

        // Create a small test image.
        QImage img(64, 64, QImage::Format_ARGB32);
        img.fill(Qt::red);

        history.push(img);

        QCOMPARE(history.items().size(), static_cast<size_t>(1));
        QVERIFY(history.items().front().isImage());
        QVERIFY(!history.items().front().png_data.isEmpty());
        QVERIFY(!history.items().front().thumbnail.isNull());
        QVERIFY(history.items().front().byte_size > 0);
    }

    void testPngBridgeCapture()
    {
        core::ClipboardHistory history;
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        QByteArray png;
        QBuffer buffer(&png);
        QVERIFY(buffer.open(QIODevice::WriteOnly));
        QVERIFY(image.save(&buffer, "PNG"));

        history.pushPng(png);
        QCOMPARE(history.items().size(), size_t(1));
        QCOMPARE(history.items().front().png_data, png);
        QVERIFY(!history.items().front().thumbnail.isNull());
        history.pushPng(png);
        QCOMPARE(history.items().size(), size_t(1));

        history.pushPng(QByteArray(core::MemoryPolicy::MAX_IMAGE_BYTES + 1, 'x'));
        QCOMPARE(history.items().size(), size_t(1));
    }

    void testByteBudgetTracking()
    {
        core::ClipboardHistory history;
        history.push("hello");
        QVERIFY(history.totalBytes() > 0);
        QVERIFY(history.totalBytes() <= core::MemoryPolicy::MAX_TOTAL_BYTES);
    }

    void testNullImageIgnored()
    {
        core::ClipboardHistory history;
        history.push(QImage{});
        QCOMPARE(history.items().size(), static_cast<size_t>(0));
    }

    void testPinnedTextSurvivesRestartAndEviction()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString id;
        {
            core::ClipboardHistory history(dir.path());
            history.push("keep me");
            id = history.items().front().id;
            QVERIFY(history.pin(id));
            QVERIFY(history.edit(id, "edited text"));
            for (int i = 0; i < 15; ++i)
                history.push(QString("recent %1").arg(i));
            QCOMPARE(history.pinnedItems().size(), size_t(1));
            QCOMPARE(history.items().size(), size_t(10));
        }
        core::ClipboardHistory restored(dir.path());
        QCOMPARE(restored.pinnedItems().size(), size_t(1));
        QCOMPARE(restored.itemForPaste(id).text, QString("edited text"));
        QVERIFY(restored.unpin(id));
        QCOMPARE(restored.items().front().text, QString("edited text"));
        QVERIFY(restored.remove(id));
        QVERIFY(restored.find(id) == nullptr);
        core::ClipboardHistory again(dir.path());
        QVERIFY(again.pinnedItems().empty());
    }

    void testPinnedImageNoteAndLazyPayload()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::green);
        QByteArray png;
        QBuffer buffer(&png);
        QVERIFY(buffer.open(QIODevice::WriteOnly));
        QVERIFY(image.save(&buffer, "PNG"));
        QString id;
        {
            core::ClipboardHistory history(dir.path());
            history.pushPng(png);
            id = history.items().front().id;
            QVERIFY(history.pin(id));
            QVERIFY(history.edit(id, "Screenshot of chart"));
            QCOMPARE(history.pinnedItems().front().png_data.size(), qsizetype(0));
            QCOMPARE(history.itemForPaste(id).png_data, png);
        }
        core::ClipboardHistory restored(dir.path());
        QCOMPARE(restored.pinnedItems().size(), size_t(1));
        QCOMPARE(restored.pinnedItems().front().note, QString("Screenshot of chart"));
        QVERIFY(restored.pinnedItems().front().png_data.isEmpty());
        QCOMPARE(restored.itemForPaste(id).png_data, png);
        QVERIFY(restored.remove(id));
        core::ClipboardHistory again(dir.path());
        QVERIFY(again.pinnedItems().empty());
    }

    void testEditLimits()
    {
        core::ClipboardHistory history;
        history.push("original");
        const QString id = history.items().front().id;
        QVERIFY(!history.edit(id, ""));
        QCOMPARE(history.items().front().text, QString("original"));
        QVERIFY(!history.edit(id, QString(40000, 'x')));
        QCOMPARE(history.items().front().text, QString("original"));
    }

    void testPinnedLimitDoesNotEvictPins()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        core::ClipboardHistory history(dir.path());
        for (int i = 0; i < 10; ++i)
        {
            history.push(QString("pin %1").arg(i));
            QVERIFY(history.pin(history.items().front().id));
        }
        history.push("extra");
        QVERIFY(!history.pin(history.items().front().id));
        QCOMPARE(history.pinnedItems().size(), size_t(10));
        QCOMPARE(history.items().size(), size_t(1));
        core::ClipboardHistory restored(dir.path());
        QCOMPARE(restored.pinnedItems().size(), size_t(10));
    }

    void testImageNoteSizeLimit()
    {
        core::ClipboardHistory history;
        QImage image(16, 16, QImage::Format_ARGB32);
        image.fill(Qt::red);
        history.push(image);
        const QString id = history.items().front().id;
        QVERIFY(history.edit(id, "note"));
        QCOMPARE(history.items().front().note, QString("note"));
        QVERIFY(!history.edit(id, QString(1025, 'x')));
        QCOMPARE(history.items().front().note, QString("note"));
    }
};

QTEST_MAIN(TestHistory)
#include "test_history.moc"
