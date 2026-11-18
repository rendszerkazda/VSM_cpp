#include "vsm/SupabaseStorage.h"

#include <cctype>
#include <stdexcept>
#include <utility>

#include "vsm/Settings.h"

namespace vsm {

namespace {

std::string percentEncode(const std::string& text) {
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : text) {
        if (std::isalnum(c) != 0 || c == '-' || c == '_' || c == '.' || c == '~') {
            out += static_cast<char>(c);
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 0x0F];
        }
    }
    return out;
}

std::string bodyText(const Bytes& body) {
    return std::string(body.begin(), body.end());
}

}  // namespace

SupabaseStorage::SupabaseStorage(IHttpClient& http, std::string baseUrl, std::string apiKey,
                                 std::string bucket)
    : http_(http),
      baseUrl_(std::move(baseUrl)),
      apiKey_(std::move(apiKey)),
      bucket_(std::move(bucket)) {
    if (baseUrl_.empty() || apiKey_.empty() || bucket_.empty()) {
        throw std::runtime_error(
            "Supabase is not configured: set supabase_url (or supabase_project_id), "
            "supabase_key (or VSM_SUPABASE_KEY) and bucket.");
    }
    while (!baseUrl_.empty() && baseUrl_.back() == '/') {
        baseUrl_.pop_back();
    }
}

std::string SupabaseStorage::objectUrl(const std::string& name) const {
    if (!isValidName(name)) {
        throw std::runtime_error("Invalid object name: " + name);
    }
    return baseUrl_ + "/storage/v1/object/" + percentEncode(bucket_) + "/" + percentEncode(name);
}

std::vector<std::string> SupabaseStorage::authHeaders() const {
    return {"apikey: " + apiKey_, "Authorization: Bearer " + apiKey_};
}

void SupabaseStorage::put(const std::string& name, const Bytes& data) {
    auto headers = authHeaders();
    headers.push_back("Content-Type: application/octet-stream");
    headers.push_back("x-upsert: true");

    const HttpResponse response = http_.request("POST", objectUrl(name), headers, data);
    if (response.status != 200 && response.status != 201) {
        throw std::runtime_error("Upload of " + name + " failed (HTTP " +
                                 std::to_string(response.status) + "): " + bodyText(response.body));
    }
}

std::optional<Bytes> SupabaseStorage::get(const std::string& name) {
    const HttpResponse response = http_.request("GET", objectUrl(name), authHeaders(), {});
    if (response.status == 200) {
        return response.body;
    }
    const std::string text = bodyText(response.body);
    const bool notFound =
        response.status == 404 ||
        (response.status == 400 &&
         (text.find("not_found") != std::string::npos || text.find("404") != std::string::npos ||
          text.find("not found") != std::string::npos));
    if (notFound) {
        return std::nullopt;
    }
    throw std::runtime_error("Download of " + name + " failed (HTTP " +
                             std::to_string(response.status) + "): " + text);
}

}  // namespace vsm
