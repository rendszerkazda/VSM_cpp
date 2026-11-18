#pragma once

/**
 * @file CurlProcessHttpClient.h
 * @brief IHttpClient implementation that runs the "curl" command line tool.
 */

#include "vsm/IHttpClient.h"

namespace vsm {

/**
 * @brief Performs HTTP requests by running the curl executable.
 *
 * curl.exe ships with Windows 10 and later and is available on Linux and
 * macOS, so no extra library has to be built. Headers and bodies are passed
 * through temporary files, so secrets never appear on the command line.
 */
class CurlProcessHttpClient : public IHttpClient {
public:
    /** @copydoc IHttpClient::request */
    HttpResponse request(const std::string& method, const std::string& url,
                         const std::vector<std::string>& headers, const Bytes& body) override;
};

}  // namespace vsm
