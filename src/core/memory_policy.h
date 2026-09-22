#pragma once

#include <QtGlobal>

namespace core {

struct MemoryPolicy
{
    // Maximum number of entries regardless of type.
    static constexpr size_t    MAX_ITEMS       = 10;

    // Per-item caps (compressed bytes).
    static constexpr qsizetype MAX_TEXT_BYTES  =  64 * 1024;        //  64 KB
    static constexpr qsizetype MAX_IMAGE_BYTES =   4 * 1024 * 1024; //   4 MB (PNG-compressed)

    // Hard ceiling across all items combined.
    static constexpr qsizetype MAX_TOTAL_BYTES =  32 * 1024 * 1024; //  32 MB
};

} // namespace core
