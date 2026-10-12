#include "global_hotkey.h"
#include "global_hotkey_socket.h"
#include "app/logging.h"

namespace platform {

std::unique_ptr<GlobalHotkey> GlobalHotkey::create(QObject* parent)
{
    app::log_info("GlobalHotkey: using socket trigger");
    return std::make_unique<GlobalHotkeySocket>(parent);
}

} // namespace platform
