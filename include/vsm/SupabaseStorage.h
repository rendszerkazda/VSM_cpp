#pragma once

/**
 * @file SupabaseStorage.h
 * @brief IStorage implementation backed by Supabase Storage (REST API).
 */

#include <string>

#include "vsm/IHttpClient.h"
#include "vsm/IStorage.h"

namespace vsm {

/**
 * @brief Stores objects in a Supabase Storage bucket.
 *
 * Uses the documented REST endpoints
 * `POST/GET {url}/storage/v1/object/{bucket}/{name}`. The HTTP transport is
 * injected, so the class can be tested without network.
 */
class SupabaseStorage : public IStorage {
public:
    /**
     * @brief Creates the storage.
     *
     * @param http HTTP client used for all requests (must outlive this object).
     * @param baseUrl Project URL, for example "https://xyz.supabase.co".
     * @param apiKey Supabase API key.
     * @param bucket Bucket name.
     */
    SupabaseStorage(IHttpClient& http, std::string baseUrl, std::string apiKey, std::string bucket);

    /** @copydoc IStorage::put */
    void put(const std::string& name, const Bytes& data) override;

    /** @copydoc IStorage::get */
    std::optional<Bytes> get(const std::string& name) override;

private:
    /** @brief Builds the object URL for a name (percent-encoded). */
    std::string objectUrl(const std::string& name) const;
    /** @brief Builds the authentication headers. */
    std::vector<std::string> authHeaders() const;

    IHttpClient& http_;
    std::string baseUrl_;
    std::string apiKey_;
    std::string bucket_;
};

}  // namespace vsm
