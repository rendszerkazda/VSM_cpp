#include <gtest/gtest.h>

#include "vsm/GzipCompressor.h"

using vsm::Bytes;

TEST(GzipCompressorTest, RoundTripRestoresOriginalData) {
    vsm::GzipCompressor gz;
    Bytes data;
    for (int i = 0; i < 100000; ++i) {
        data.push_back(static_cast<std::uint8_t>((i * 7) % 251));
    }
    EXPECT_EQ(gz.decompress(gz.compress(data)), data);
}

TEST(GzipCompressorTest, RoundTripOfEmptyData) {
    vsm::GzipCompressor gz;
    EXPECT_TRUE(gz.decompress(gz.compress({})).empty());
}

TEST(GzipCompressorTest, RepetitiveDataBecomesSmaller) {
    vsm::GzipCompressor gz;
    const Bytes data(50000, 42);
    EXPECT_LT(gz.compress(data).size(), data.size() / 10);
}

TEST(GzipCompressorTest, OutputHasGzipMagicBytes) {
    vsm::GzipCompressor gz;
    const Bytes out = gz.compress({1, 2, 3});
    ASSERT_GE(out.size(), 2u);
    EXPECT_EQ(out[0], 0x1f);
    EXPECT_EQ(out[1], 0x8b);
}

TEST(GzipCompressorTest, GarbageInputThrows) {
    vsm::GzipCompressor gz;
    EXPECT_THROW(gz.decompress({1, 2, 3, 4, 5}), std::runtime_error);
}

TEST(GzipCompressorTest, TruncatedInputThrows) {
    vsm::GzipCompressor gz;
    Bytes out = gz.compress(Bytes(1000, 9));
    out.resize(out.size() / 2);
    EXPECT_THROW(gz.decompress(out), std::runtime_error);
}
