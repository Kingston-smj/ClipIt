#pragma once

#include <QString>
#include <QXmlStreamReader>

namespace platform {

// Check for ClipIt's exported object, not just GNOME Shell's bus name.
// The standard D-Bus Peer.Ping also answers for nonexistent object paths.
inline bool hasShellPasteMethod(const QString& introspection)
{
    QXmlStreamReader xml(introspection);
    bool in_shell_interface = false;
    bool has_paste = false;
    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("interface"))
            in_shell_interface =
                xml.attributes().value(QLatin1String("name")) ==
                QLatin1String("org.clipit.Shell");
        else if (in_shell_interface && xml.isStartElement() &&
                 xml.name() == QLatin1String("method") &&
                 xml.attributes().value(QLatin1String("name")) ==
                 QLatin1String("Paste"))
            has_paste = true;
        else if (xml.isEndElement() && xml.name() == QLatin1String("interface"))
            in_shell_interface = false;
    }
    return !xml.hasError() && has_paste;
}

} // namespace platform
