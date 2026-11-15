#pragma once

/**
 * @file FileUtil.h
 * @brief Helpers for reading and writing whole binary files.
 */

#include <filesystem>

#include "vsm/Common.h"

namespace vsm {

/**
 * @brief Reads a whole file into memory.
 * @param path File to read.
 * @return The file contents.
 * @throws std::runtime_error if the file cannot be read.
 */
Bytes readFile(const std::filesystem::path& path);

/**
 * @brief Writes data to a file, creating parent directories as needed.
 * @param path Destination file (overwritten if it exists).
 * @param data The bytes to write.
 * @throws std::runtime_error if the file cannot be written.
 */
void writeFile(const std::filesystem::path& path, const Bytes& data);

}  // namespace vsm
