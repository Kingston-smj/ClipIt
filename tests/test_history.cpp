#include <QTest>
#include "core/clipboard_history.h"

class TestHistory : public QObject
{
    Q_OBJECT

private slots:
    void testPushSingle()
    {
        core::ClipboardHistory history;
        history.push("hello");
        QCOMPARE(history.items().size(), static_cast<size_t>(1));
        QCOMPARE(history.items().front(), QString("hello"));
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

        // Re-push "alpha"
        history.push("alpha");

        QCOMPARE(history.items().size(), static_cast<size_t>(3));
        QCOMPARE(history.items()[0], QString("alpha"));
        QCOMPARE(history.items()[1], QString("gamma"));
        QCOMPARE(history.items()[2], QString("beta"));
    }

    void testMaxCapacity()
    {
        core::ClipboardHistory history;
        for (int i = 0; i < 15; ++i)
        {
            history.push(QString("item_%1").arg(i));
        }

        QCOMPARE(history.items().size(), static_cast<size_t>(10));
        QCOMPARE(history.items().front(), QString("item_14"));
        QCOMPARE(history.items().back(), QString("item_5"));
    }
};

QTEST_MAIN(TestHistory)
#include "test_history.moc"
