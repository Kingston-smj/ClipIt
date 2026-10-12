#include <QTest>
#include <QImage>
#include <QBuffer>
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
};

QTEST_MAIN(TestHistory)
#include "test_history.moc"
