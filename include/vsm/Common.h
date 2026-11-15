#pragma once

/**
 * @file Common.h
 * @brief Small shared type definitions.
 */

#include <cstdint>
#include <vector>

namespace vsm {

/** @brief A block of raw binary data (file contents, compressed data...). */
using Bytes = std::vector<std::uint8_t>;

}  // namespace vsm
