#include "ui/emoji_panel.h"
#include "ui/kaomoji_panel.h"
#include "ui/symbol_panel.h"

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSignalSpy>
#include <QTabBar>
#include <QTest>

class TestPickerCategories : public QObject
{
    Q_OBJECT

private:
    void checkPanel(ui::PickerPanel& panel, const QString& searchText,
                    const QString& expectedGlyph, const QString& firstCategoryGlyph)
    {
        auto* categories = panel.findChild<QTabBar*>("pickerCategories");
        auto* heading = panel.findChild<QLabel*>("pickerHeading");
        auto* search = panel.findChild<QLineEdit*>();
        auto* grid = panel.findChild<QListWidget*>();
        QVERIFY(categories);
        QVERIFY(heading);
        QVERIFY(search);
        QVERIFY(grid);
        QVERIFY(categories->count() > 1);
        QCOMPARE(categories->tabText(0), firstCategoryGlyph);
        for (int i = 0; i < categories->count(); ++i)
        {
            const QString name = categories->tabData(i).toString();
            QVERIFY(!name.isEmpty());
            QVERIFY(categories->tabText(i) != name);
            QCOMPARE(categories->tabToolTip(i), name);
            QCOMPARE(categories->accessibleTabName(i), name);
        }

        panel.show();
        panel.onShown();
        QCoreApplication::processEvents();
        const int firstCount = grid->count();
        QVERIFY(firstCount > 0);
        QCOMPARE(heading->text(), categories->tabData(0).toString());

        categories->setCurrentIndex(1);
        QVERIFY(grid->count() > 0);
        QCOMPARE(heading->text(), categories->tabData(1).toString());

        search->setText(searchText);
        QCOMPARE(heading->text(), QStringLiteral("Search results"));
        QListWidgetItem* match = nullptr;
        for (int i = 0; i < grid->count(); ++i)
        {
            if (grid->item(i)->text() == expectedGlyph)
                match = grid->item(i);
        }
        QVERIFY(match);

        QSignalSpy selected(&panel, &ui::PickerPanel::characterSelected);
        QTest::mouseClick(grid->viewport(), Qt::LeftButton, {},
                          grid->visualItemRect(match).center());
        QCOMPARE(selected.size(), 1);
        QCOMPARE(selected.first().first().toString(), expectedGlyph);

        search->clear();
        QCOMPARE(heading->text(), categories->tabData(1).toString());
        QVERIFY(grid->count() > 0);
    }

private slots:
    void emoji()
    {
        ui::EmojiPanel panel;
        checkPanel(panel, QStringLiteral("trophy"), QStringLiteral("🏆"),
                   QStringLiteral("😀"));
    }

    void kaomoji()
    {
        ui::KaomojiPanel panel;
        checkPanel(panel, QStringLiteral("loving"), QStringLiteral("(◕‿◕)♡"),
                   QStringLiteral("(^_^)"));
    }

    void symbols()
    {
        ui::SymbolPanel panel;
        checkPanel(panel, QStringLiteral("quarter note"), QStringLiteral("♩"),
                   QStringLiteral("→"));
    }
};

QTEST_MAIN(TestPickerCategories)
#include "test_picker_categories.moc"
