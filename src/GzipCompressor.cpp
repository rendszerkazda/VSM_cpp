#include "vsm/GzipCompressor.h"

#include <zlib.h>

#include <array>
#include <stdexcept>

namespace vsm {

namespace {

constexpr int kGzipWindowBits = 15 + 16;   // 16 selects the gzip wrapper
constexpr int kAutoDetectWindowBits = 15 + 32;  // gzip or zlib header
constexpr std::size_t kChunk = 64 * 1024;

}  // namespace

GzipCompressor::GzipCompressor(int level) : level_(level) {}

Bytes GzipCompressor::compress(const Bytes& input) {
    z_stream stream{};
    if (deflateInit2(&stream, level_, Z_DEFLATED, kGzipWindowBits, 8, Z_DEFAULT_STRATEGY) !=
        Z_OK) {
        throw std::runtime_error("gzip: deflateInit2 failed");
    }

    stream.next_in = const_cast<Bytef*>(input.data());
    stream.avail_in = static_cast<uInt>(input.size());

    Bytes output;
    std::array<Bytef, kChunk> buffer;
    int status = Z_OK;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = deflate(&stream, Z_FINISH);
        if (status == Z_STREAM_ERROR) {
            deflateEnd(&stream);
            throw std::runtime_error("gzip: compression failed");
        }
        output.insert(output.end(), buffer.data(), buffer.data() + (buffer.size() - stream.avail_out));
    } while (status != Z_STREAM_END);

    deflateEnd(&stream);
    return output;
}

Bytes GzipCompressor::decompress(const Bytes& input) {
    z_stream stream{};
    if (inflateInit2(&stream, kAutoDetectWindowBits) != Z_OK) {
        throw std::runtime_error("gzip: inflateInit2 failed");
    }

    stream.next_in = const_cast<Bytef*>(input.data());
    stream.avail_in = static_cast<uInt>(input.size());

    Bytes output;
    std::array<Bytef, kChunk> buffer;
    int status = Z_OK;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = inflate(&stream, Z_NO_FLUSH);
        if (status != Z_OK && status != Z_STREAM_END) {
            inflateEnd(&stream);
            throw std::runtime_error("gzip: corrupt or truncated data");
        }
        output.insert(output.end(), buffer.data(), buffer.data() + (buffer.size() - stream.avail_out));
        if (status == Z_OK && stream.avail_in == 0 && stream.avail_out != 0) {
            inflateEnd(&stream);
            throw std::runtime_error("gzip: truncated data");
        }
    } while (status != Z_STREAM_END);

    inflateEnd(&stream);
    return output;
}

}  // namespace vsm
