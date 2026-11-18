#include <gtest/gtest.h>

#include <algorithm>

#include "FakeHttpClient.h"
#include "vsm/SupabaseStorage.h"

namespace {

bool hasHeader(const FakeHttpClient::Call& call, const std::string& header) {
    return std::find(call.headers.begin(), call.headers.end(), header) != call.headers.end();
}

}  // namespace

TEST(SupabaseStorageTest, PutPostsToObjectUrlWithAuthAndUpsert) {
    FakeHttpClient http;
    http.response.status = 200;
    vsm::SupabaseStorage storage(http, "https://p.supabase.co/", "KEY", "valheim-saves");

    storage.put("Arena19_db.gz", {1, 2, 3});

    ASSERT_EQ(http.calls.size(), 1u);
    const auto& call = http.calls[0];
    EXPECT_EQ(call.method, "POST");
    EXPECT_EQ(call.url, "https://p.supabase.co/storage/v1/object/valheim-saves/Arena19_db.gz");
    EXPECT_TRUE(hasHeader(call, "apikey: KEY"));
    EXPECT_TRUE(hasHeader(call, "Authorization: Bearer KEY"));
    EXPECT_TRUE(hasHeader(call, "x-upsert: true"));
    EXPECT_EQ(call.body, (vsm::Bytes{1, 2, 3}));
}

TEST(SupabaseStorageTest, PutThrowsOnHttpError) {
    FakeHttpClient http;
    http.response.status = 403;
    http.response.body = {'n', 'o'};
    vsm::SupabaseStorage storage(http, "https://p.supabase.co", "KEY", "b");
    EXPECT_THROW(storage.put("a.gz", {1}), std::runtime_error);
}

TEST(SupabaseStorageTest, GetReturnsBodyOn200) {
    FakeHttpClient http;
    http.response.status = 200;
    http.response.body = {9, 8};
    vsm::SupabaseStorage storage(http, "https://p.supabase.co", "KEY", "b");

    const auto result = storage.get("a.gz");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, (vsm::Bytes{9, 8}));
    EXPECT_EQ(http.calls[0].method, "GET");
}

TEST(SupabaseStorageTest, GetReturnsNulloptWhenObjectIsMissing) {
    FakeHttpClient http;
    http.response.status = 404;
    vsm::SupabaseStorage storage(http, "https://p.supabase.co", "KEY", "b");
    EXPECT_FALSE(storage.get("a.gz").has_value());

    const std::string msg = R"({"statusCode":"404","error":"not_found"})";
    http.response.status = 400;
    http.response.body.assign(msg.begin(), msg.end());
    EXPECT_FALSE(storage.get("a.gz").has_value());
}

TEST(SupabaseStorageTest, GetThrowsOnOtherErrors) {
    FakeHttpClient http;
    http.response.status = 500;
    vsm::SupabaseStorage storage(http, "https://p.supabase.co", "KEY", "b");
    EXPECT_THROW(storage.get("a.gz"), std::runtime_error);
}

TEST(SupabaseStorageTest, NamesAreValidatedAndEncoded) {
    FakeHttpClient http;
    http.response.status = 200;
    vsm::SupabaseStorage storage(http, "https://p.supabase.co", "KEY", "b");

    EXPECT_THROW(storage.put("../x", {1}), std::runtime_error);
    storage.put("My World_db.gz", {1});
    EXPECT_EQ(http.calls.back().url,
              "https://p.supabase.co/storage/v1/object/b/My%20World_db.gz");
}

TEST(SupabaseStorageTest, MissingConfigurationThrows) {
    FakeHttpClient http;
    EXPECT_THROW(vsm::SupabaseStorage(http, "", "KEY", "b"), std::runtime_error);
    EXPECT_THROW(vsm::SupabaseStorage(http, "https://p.supabase.co", "", "b"), std::runtime_error);
}
