#include <QTest>
#include "platform/gnome_shell_probe.h"

class TestShellProbe : public QObject
{
    Q_OBJECT

private slots:
    void installedExtension()
    {
        const QString xml = QStringLiteral(
            "<node><interface name='org.clipit.Shell'>"
            "<method name='Paste'/></interface></node>");
        QVERIFY(platform::hasShellPasteMethod(xml));
    }

    void absentOrInvalidExtension()
    {
        QVERIFY(!platform::hasShellPasteMethod(QStringLiteral("<node/>")));
        QVERIFY(!platform::hasShellPasteMethod(QStringLiteral(
            "<node><interface name='org.clipit.Shell'>"
            "<method name='Ping'/></interface></node>")));
        QVERIFY(!platform::hasShellPasteMethod(QStringLiteral(
            "<node><interface name='org.other.Shell'>"
            "<method name='Paste'/></interface></node>")));
        QVERIFY(!platform::hasShellPasteMethod(QStringLiteral(
            "<node><interface name='org.clipit.Shell'>")));
    }
};

QTEST_APPLESS_MAIN(TestShellProbe)
#include "test_shell_probe.moc"
