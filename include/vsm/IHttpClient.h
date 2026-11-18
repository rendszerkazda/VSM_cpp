#pragma once

/**
 * @file IHttpClient.h
 * @brief Abstract HTTP client, so network code can be replaced in tests.
 */

#include <string>
#include <vector>

#include "vsm/Common.h"

namespace vsm {

/** @brief The result of an HTTP request. */
struct HttpResponse {
    /** @brief HTTP status code, for example 200 or 404. */
    int status = 0;
    /** @brief Response body. */
    Bytes body;
};

/**
 * @brief Interface of a minimal synchronous HTTP client.
 *
 * SupabaseStorage depends on this abstraction and not on a concrete
 * network library (Dependency Inversion), so its tests run without network.
 */
class IHttpClient {
public:
    virtual ~IHttpClient() = default;

    /**
     * @brief Sends an HTTP request.
     *
     * @param method HTTP method, for example "GET" or "POST".
     * @param url Full URL.
     * @param headers Header lines in the form "Name: value".
     * @param body Request body (may be empty).
     * @return The response.
     * @throws std::runtime_error if the request could not be performed at all.
     */
    virtual HttpResponse request(const std::string& method, const std::string& url,
                                 const std::vector<std::string>& headers, const Bytes& body) = 0;
};

}  // namespace vsm
