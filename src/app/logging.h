#pragma once

// Minimal logging helpers — wraps qDebug/qWarning so the platform layer
// doesn't need to pull in <QDebug> directly everywhere.

#include <cstdio>

namespace app {

template<typename... Args>
inline void log_info(const char* fmt, Args&&... args)
{
    std::fputs("[clipit] INFO  ", stdout);
    if constexpr (sizeof...(args) == 0)
        std::fputs(fmt, stdout);
    else
        // NOLINTNEXTLINE(clang-diagnostic-format-security)
        std::fprintf(stdout, fmt, std::forward<Args>(args)...);
    std::fputc('\n', stdout);
}

template<typename... Args>
inline void log_warn(const char* fmt, Args&&... args)
{
    std::fputs("[clipit] WARN  ", stderr);
    if constexpr (sizeof...(args) == 0)
        std::fputs(fmt, stderr);
    else
        // NOLINTNEXTLINE(clang-diagnostic-format-security)
        std::fprintf(stderr, fmt, std::forward<Args>(args)...);
    std::fputc('\n', stderr);
}

} // namespace app
