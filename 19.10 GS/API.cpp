#include "pch.h"
#include "API.h"

#include <mutex>

static bool bCurlInitialized = false;
static std::mutex curlInitMutex;

void InitCurl() {
    std::lock_guard<std::mutex> lock(curlInitMutex);
    if (!bCurlInitialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        bCurlInitialized = true;
    }
}

size_t API::CallBack(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

void API::SendGet(const std::string& url) {
    std::thread([url = url]() {
        InitCurl();
        CURL* curl = curl_easy_init();
        if (!curl) {
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallBack);

        std::string body;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }).detach();
}

std::string API::GetResponse(const std::string& URL) {
    InitCurl();
    CURL* Curl = curl_easy_init();
    std::string response;

    if (Curl) {
        curl_easy_setopt(Curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CallBack);
        curl_easy_setopt(Curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(Curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(Curl);
        if (res != CURLE_OK) {
            response = "";
        }

        curl_easy_cleanup(Curl);
    }

    return response;
}

std::string API::GameServer(const std::string& url, const std::string& ip, int port)
{
    CURL* Curl;
    CURLcode res;
    std::string returning;

    InitCurl();
    Curl = curl_easy_init();

    if (Curl) {
        std::string responseData = "{ \"ipAddress\": \"" + ip + "\", \"port\": " + std::to_string(port) + " }";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(Curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, responseData.c_str());
        curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, +[](void* contents, size_t size, size_t nmemb, std::string* output) {
            size_t totalSize = size * nmemb;
            output->append((char*)contents, totalSize);
            return totalSize;
        });

        curl_easy_setopt(Curl, CURLOPT_WRITEDATA, &returning);

        res = curl_easy_perform(Curl);

        if (res != CURLE_OK) {
            returning = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(Curl);
    }

    return returning;
}

void API::GameServerAsync(const std::string& url, const std::string& ip, int port)
{
    std::thread([url, ip, port]() {
        GameServer(url, ip, port);
    }).detach();
}
