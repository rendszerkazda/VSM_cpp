#pragma once

#include <string>
#include <vector>

#include "vsm/IHttpClient.h"

/** @brief Test double that records requests and returns a canned response. */
class FakeHttpClient : public vsm::IHttpClient {
public:
    struct Call {
        std::string method;
        std::string url;
        std::vector<std::string> headers;
        vsm::Bytes body;
    };

    vsm::HttpResponse response;
    std::vector<Call> calls;

    vsm::HttpResponse request(const std::string& method, const std::string& url,
                              const std::vector<std::string>& headers,
                              const vsm::Bytes& body) override {
        calls.push_back({method, url, headers, body});
        return response;
    }
};
