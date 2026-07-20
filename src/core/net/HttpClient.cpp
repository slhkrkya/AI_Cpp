#include "core/net/HttpClient.h"

#include <curl/curl.h>

#include <mutex>

namespace aicpp::net {

namespace {

void ensureGlobalInit() {
    static std::once_flag once;
    std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

struct WriteContext {
    const ChunkCallback* onChunk;
    std::atomic<bool>* cancel;
};

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<WriteContext*>(userdata);
    if (ctx->cancel && ctx->cancel->load()) {
        return 0;  // aborts the transfer
    }
    const size_t total = size * nmemb;
    if (ctx->onChunk && *ctx->onChunk) {
        (*ctx->onChunk)(std::string_view(ptr, total));
    }
    return total;
}

}  // namespace

HttpClient::HttpClient() { ensureGlobalInit(); }
HttpClient::~HttpClient() = default;

HttpResponse HttpClient::postStreaming(const std::string& url,
                                         const std::vector<std::string>& headers,
                                         const std::string& body,
                                         const ChunkCallback& onChunk,
                                         std::atomic<bool>* cancel) {
    HttpResponse response;

    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "curl_easy_init failed";
        return response;
    }

    struct curl_slist* headerList = nullptr;
    for (const auto& h : headers) {
        headerList = curl_slist_append(headerList, h.c_str());
    }

    WriteContext ctx{&onChunk, cancel};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);       // streaming: no overall timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.status_code = httpCode;
    }

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    return response;
}

}  // namespace aicpp::net
