#pragma once

// ClipIt messages use Qt's logging path so both application and Qt warnings
// reach the same timestamped file.

#include <QtLogging>

namespace app {

void install_logging();

template<typename... Args>
inline void log_info(const char* fmt, Args... args)
{
    if constexpr (sizeof...(args) == 0)
        qInfo("%s", fmt);
    else
        qInfo(fmt, args...);
}

template<typename... Args>
inline void log_warn(const char* fmt, Args... args)
{
    if constexpr (sizeof...(args) == 0)
        qWarning("%s", fmt);
    else
        qWarning(fmt, args...);
}

template<typename... Args>
inline void log_error(const char* fmt, Args... args)
{
    if constexpr (sizeof...(args) == 0)
        qCritical("%s", fmt);
    else
        qCritical(fmt, args...);
}

} // namespace app
