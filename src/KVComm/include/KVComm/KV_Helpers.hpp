#pragma once

#include <stddef.h>  // size_t

/**
 * @file  
 * @brief   Helpers for address manipulation used to layout dictionary entries 
 *          in memory.
 */

/// Get the offset of the next 4-byte word.
///
///     in:   0  1  2  3  4  5  6  7  8  ...
///     out:  4  4  4  4  8  8  8  8  12 ...
inline size_t nextWord(size_t i) { return i - (i % 4) + 4; }

/// Round up a size to a multiple of 4-byte words.
///
///     in:   0  1  2  3  4  5  6  7  8  9  ...
///     out:  0  4  4  4  4  8  8  8  8  12 ...
inline size_t roundUpToWordSizeMultiple(size_t i) {
    return i + 3 - ((i + 3) % 4);
}