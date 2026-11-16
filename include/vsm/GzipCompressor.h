#pragma once

/**
 * @file GzipCompressor.h
 * @brief ICompressor implementation using gzip (zlib).
 */

#include "vsm/ICompressor.h"

namespace vsm {

/**
 * @brief Compresses to and from the standard gzip format.
 *
 * The output is a normal .gz stream, so it is compatible with the files
 * produced by the earlier Python version of the save manager.
 */
class GzipCompressor : public ICompressor {
public:
    /**
     * @brief Creates the compressor.
     * @param level zlib compression level, 1 (fast) to 9 (small). Default 6.
     */
    explicit GzipCompressor(int level = 6);

    /** @copydoc ICompressor::compress */
    Bytes compress(const Bytes& input) override;

    /** @copydoc ICompressor::decompress */
    Bytes decompress(const Bytes& input) override;

private:
    /** @brief zlib compression level. */
    int level_;
};

}  // namespace vsm
