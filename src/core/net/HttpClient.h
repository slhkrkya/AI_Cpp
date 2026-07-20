#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace aicpp::net {

struct HttpResponse {
    long status_code = 0;
    std::string error;  // non-empty on a transport-level failure (DNS, connect, etc.)

    bool ok() const { return error.empty() && status_code >= 200 && status_code < 300; }
};

using ChunkCallback = std::function<void(std::string_view)>;

// Thin libcurl wrapper. One instance is safe to reuse sequentially; not
// intended to be shared across concurrent requests.
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpResponse postStreaming(const std::string& url,
                                const std::vector<std::string>& headers,
                                const std::string& body,
                                const ChunkCallback& onChunk,
                                std::atomic<bool>* cancel = nullptr);
};

}  // namespace aicpp::net
