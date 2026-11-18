#include "vsm/CurlProcessHttpClient.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <random>
#include <stdexcept>

#include "vsm/FileUtil.h"

#ifdef _WIN32
#define VSM_POPEN _popen
#define VSM_PCLOSE _pclose
#else
#define VSM_POPEN popen
#define VSM_PCLOSE pclose
#endif

namespace vsm {

namespace {

/** @brief A temporary file path that is deleted when the object dies. */
class TempFile {
public:
    explicit TempFile(const std::string& suffix) {
        std::random_device rd;
        path_ = std::filesystem::temp_directory_path() /
                ("vsm_http_" + std::to_string(rd()) + std::to_string(rd()) + suffix);
    }
    ~TempFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

/** @brief Quotes an argument for the platform shell; rejects dangerous characters. */
std::string quote(const std::string& arg) {
    for (char c : arg) {
        if (c == '"' || c == '\'' || c == '\n' || c == '\r' || c == '`') {
            throw std::runtime_error("Unsafe character in command line argument");
        }
    }
#ifdef _WIN32
    return "\"" + arg + "\"";
#else
    return "'" + arg + "'";
#endif
}

}  // namespace

HttpResponse CurlProcessHttpClient::request(const std::string& method, const std::string& url,
                                            const std::vector<std::string>& headers,
                                            const Bytes& body) {
    TempFile headerFile(".hdr");
    TempFile bodyFile(".body");
    TempFile outFile(".out");

    std::string headerText;
    for (const auto& h : headers) {
        headerText += h + "\n";
    }
    writeFile(headerFile.path(), Bytes(headerText.begin(), headerText.end()));

    std::string command = "curl -sS -X " + quote(method) + " -H @" + quote(headerFile.path().string());
    if (!body.empty()) {
        writeFile(bodyFile.path(), body);
        command += " --data-binary @" + quote(bodyFile.path().string());
    }
    command += " -o " + quote(outFile.path().string());
    command += " -w " + quote("%{http_code}");
    command += " " + quote(url);

    FILE* pipe = VSM_POPEN(command.c_str(), "r");
    if (pipe == nullptr) {
        throw std::runtime_error("Cannot start curl. Is curl installed?");
    }
    std::string codeText;
    char buffer[64];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        codeText += buffer;
    }
    const int exitCode = VSM_PCLOSE(pipe);
    if (exitCode != 0) {
        throw std::runtime_error("curl failed (exit code " + std::to_string(exitCode) +
                                 "). Check your network connection and URL.");
    }

    HttpResponse response;
    response.status = std::atoi(codeText.c_str());
    if (std::filesystem::exists(outFile.path())) {
        response.body = readFile(outFile.path());
    }
    return response;
}

}  // namespace vsm
