#include "global_hotkey.h"
#include "global_hotkey_socket.h"
#include "app/logging.h"

#ifdef CLIPIT_X11_BACKEND
#  include "global_hotkey_x11.h"
#endif

#include <QByteArray>
#include <cstdlib>

namespace platform {

std::unique_ptr<GlobalHotkey> GlobalHotkey::create(QObject* parent)
{
#ifdef CLIPIT_X11_BACKEND
    const char* session = std::getenv("XDG_SESSION_TYPE");
    if (session && QByteArray(session).toLower() == "x11")
    {
        app::log_info("GlobalHotkey: X11 session detected — using XGrabKey + socket backends");

        // Return X11 backend; it will also start the socket internally so both
        // trigger paths are active on pure X11.
        // (For simplicity we still return a single object; ApplicationController
        //  can start both independently if preferred.)
        return std::make_unique<GlobalHotkeyX11>(parent);
    }
#endif

    app::log_info("GlobalHotkey: Wayland/unknown session — using socket trigger only");
    return std::make_unique<GlobalHotkeySocket>(parent);
}

} // namespace platform
