#pragma once

/**
 * @file ICompressor.h
 * @brief Abstract compression algorithm (Strategy pattern).
 */

#include "vsm/Common.h"

namespace vsm {

/**
 * @brief Interface of a compression algorithm.
 *
 * Another Strategy: the upload/download logic does not care how the data is
 * compressed, only that it can be compressed and decompressed again.
 */
class ICompressor {
public:
    virtual ~ICompressor() = default;

    /**
     * @brief Compresses data.
     * @param input Raw data.
     * @return The compressed data.
     * @throws std::runtime_error if compression fails.
     */
    virtual Bytes compress(const Bytes& input) = 0;

    /**
     * @brief Decompresses data produced by compress().
     * @param input Compressed data.
     * @return The original data.
     * @throws std::runtime_error if the input is corrupt.
     */
    virtual Bytes decompress(const Bytes& input) = 0;
};

}  // namespace vsm
