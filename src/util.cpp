#include "pch.h"

HTTPClient::~HTTPClient()
{
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    session = connect = request = 0;
}

bool HTTPClient::Init(const std::wstring hostname, unsigned short port, bool https, int timeout, std::optional<std::wstring> agent)
{
    debuglog("%s -> %ls\n", https ? "https" : "http", hostname.c_str());
    // use WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY? Win8.1+ only
     // flag WINHTTP_FLAG_ASYNC exists, might be useful
    session = WinHttpOpen(agent.has_value() ? agent.value().c_str() : 0, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 /* flags */);
    if (!session) {
        debuglog("%s failed: %08X\n", "WinHttpOpen", lastError = GetLastError());
        return false;
    }

    if (WinHttpSetStatusCallback(session, statusCallback, WINHTTP_CALLBACK_FLAG_SECURE_FAILURE, 0) == WINHTTP_INVALID_STATUS_CALLBACK)
    {
        debuglog("%s failed: %08X\n", "Init WinHttpSetStatusCallback WINHTTP_CALLBACK_FLAG_SECURE_FAILURE", lastError = GetLastError());
        return false;
    }
    {
        DWORD_PTR ref = reinterpret_cast<DWORD_PTR>(this);
        if (!WinHttpSetOption(session, WINHTTP_OPTION_CONTEXT_VALUE, reinterpret_cast<LPVOID*>(&ref), sizeof(ref))) {
            debuglog("%s failed: %08X\n", "Init WinHttpSetOption WINHTTP_OPTION_CONTEXT_VALUE", lastError = GetLastError());
            return false;
        }
    }

    if (timeout != 0) {
        if (!WinHttpSetTimeouts(session, timeout, timeout, timeout, timeout)) {
            debuglog("%s failed: %08X\n", "WinHttpSetTimeouts", lastError = GetLastError());
            return false;
        }
    }

    if (!port) port = https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

    connect = WinHttpConnect(session, hostname.c_str(), port, 0);
    if (!connect) {
        debuglog("%s failed: %08X\n", "WinHttpConnect", lastError = GetLastError());
        return false;
    }
    isHTTPS = https;
    debuglog("%s ok\n", "WinHttpConnect");

    return true;
}

void HTTPClient::reset()
{
    // call destructor then the constructor to reinitialize the object
    // as long as both calls remain unchanged below this should be safe
    this->~HTTPClient();
    new (this) HTTPClient();
}

bool HTTPClient::GET(const std::wstring path)
{
    debuglog("GET %ls\n", path.c_str());

    if (request) {
        debuglog("closing old request\n");
        WinHttpCloseHandle(request);
        request = 0;
    }

    request = WinHttpOpenRequest(connect, L"GET", path.c_str(), 0 /* HTTP 1.1 */, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, isHTTPS ? WINHTTP_FLAG_SECURE : 0 /* flags */);
    if (!request) {
        debuglog("%s failed: %08X\n", "WinHttpOpenRequest", lastError = GetLastError());
        return false;
    }

    // if disabling CA verification is ever needed, look at this:
    // https://stackoverflow.com/questions/19338395/how-do-you-use-winhttp-to-do-ssl-with-a-self-signed-cert
    if (!WinHttpSendRequest(request, 0, 0, 0, 0, 0, 0)) {
        DWORD err = lastError = GetLastError();
        if (err == ERROR_WINHTTP_SECURE_FAILURE && secureFailFlags) {
            debuglog("%s failed: %08X %d\n", "WinHttpSendRequest", err, secureFailFlags);
            secureFailFlags = 0;
        }
        else {
            debuglog("%s failed: %08X\n", "WinHttpSendRequest", err);
        }
        return false;
    }

    if (!WinHttpReceiveResponse(request, 0)) {
        debuglog("%s failed: %08X\n", "WinHttpReceiveResponse", lastError = GetLastError());
        return false;
    }
    debuglog("GET ok\n");
    return true;
}

int HTTPClient::status()
{
    if (!request) return -1;
    int status;
    DWORD datasize = sizeof status;
    DWORD index = WINHTTP_NO_HEADER_INDEX;
    if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, reinterpret_cast<void*>(&status), &datasize, &index)) {
        return status;
    }
    return -1;
}

bool HTTPClient::data(std::vector<char>& data)
{
    size_t totalBytesRead = 0;
    data.resize(2048);
    for (;;) {
        DWORD bytesAvailable;
        if (!WinHttpQueryDataAvailable(request, &bytesAvailable)) {
            debuglog("%s failed: %08X\n", "WinHttpQueryDataAvailable", lastError = GetLastError());
            return false;
        }
        if (bytesAvailable == 0) break;

        // we're reading into memory so fail if the data is more than 128MB
        if (totalBytesRead + bytesAvailable > 1024 * 1024 * 128) {
            debuglog("HTTP response exceeds 128M\n", "WinHttpRead");
            MessageBox(0, L"HTTP response exceeds 128M", L"", MB_ICONERROR);
        }

        if ((data.size() - totalBytesRead) < bytesAvailable) {
            size_t newSize = totalBytesRead + bytesAvailable;
            // round up to nearest power of two
            newSize--;
            newSize |= newSize >> 1;
            newSize |= newSize >> 2;
            newSize |= newSize >> 4;
            newSize |= newSize >> 8;
            newSize |= newSize >> 16;
            newSize++;

            data.resize(newSize);
        }

        DWORD bytesRead;
        debuglog("WinHttpReadData %p offset %zu length %u\n", data.data(), totalBytesRead, bytesAvailable);
        if (!WinHttpReadData(request, data.data() + totalBytesRead, bytesAvailable, &bytesRead)) {
            debuglog("%s failed: %08X\n", "WinHttpReadData", lastError = GetLastError());
            return false;
        }

        totalBytesRead += bytesRead;
    }
    debuglog("received %u bytes\n", totalBytesRead);
    data.resize(totalBytesRead);
    return true;
}

bool HTTPClient::text(std::wstring& text)
{
    std::vector<char> responseBytes;
    if (!data(responseBytes)) {
        return false;
    }

    text = UTF8ToWideString(responseBytes.data(), responseBytes.size());
    return true;
}

VOID HTTPClient::statusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
    (void)dwStatusInformationLength;
    debuglog("HTTPClient::statusCallback %p %p %08X %p\n", hInternet, dwContext, dwInternetStatus, lpvStatusInformation);
    HTTPClient* http = reinterpret_cast<HTTPClient*>(dwContext);
    if (!http) return;
    if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_SECURE_FAILURE) {
        http->secureFailFlags = *(reinterpret_cast<DWORD*>(lpvStatusInformation));
        debuglog("WINHTTP_CALLBACK_STATUS_SECURE_FAILURE = %08X, %d\n", http->secureFailFlags, http->secureFailFlags);
    }
}

std::wstring UTF8ToWideString(const char* s, size_t len)
{
    std::wstring result;
    if (len == (size_t)-1) len = strlen(s);

    auto newLength = MultiByteToWideChar(CP_UTF8, 0, s, len, NULL, 0);
    if (newLength == 0) {
        return result;
    }
    result.resize(newLength);
    MultiByteToWideChar(CP_UTF8, 0, s, len, result.data(), newLength);

    return result;
}

std::string WideStringToASCII(std::wstring s)
{
    std::string result;
    result.resize(s.size());
    size_t i = 0;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c < 128) result[i++] = (char)*c;
        else result[i++] = '?';
    }
    return result;
}

bool HexStringToData(const std::wstring& str, std::vector<unsigned char>& data)
{
    if ((str.length() % 2) != 0) {
        return false;
    }
    data.resize(str.length() / 2);
    char high = -1;
    size_t idx = 0;
    for (auto it = str.cbegin(); it != str.cend(); it++) {
        int value;
        if (*it >= '0' && *it <= '9') value = *it - '0';
        else if (*it >= 'A' && *it <= 'F') value = *it - 'A' + 10;
        else if (*it >= 'a' && *it <= 'f') value = *it - 'a' + 10;
        else return false;
        if (high != -1) {
            data[idx++] = (high << 4) | (char)value;
            high = -1;
        }
        else {
            high = (char)value;
        }
    }
    return true;
}

bool HexDataToData(const std::vector<char>& hex, size_t offset, size_t length , std::vector<unsigned char>& data)
{
    if ((length % 2) != 0 || offset+length > hex.size()) {
        return false;
    }
    data.resize(length / 2);
    char high = -1;
    size_t idx = 0;
    for (size_t i = offset; i < (offset + length); i++) {
        int value;
        char c = hex.at(i);
        if (c >= '0' && c <= '9') value = c - '0';
        else if (c >= 'A' && c <= 'F') value = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') value = c - 'a' + 10;
        else return false;
        if (high != -1) {
            data[idx++] = (high << 4) | (char)value;
            high = -1;
        }
        else {
            high = (char)value;
        }
    }
    return true;
}

bool CenterWindow(HWND window)
{
    RECT windowsize, desktopsize;
    GetWindowRect(GetDesktopWindow(), &desktopsize);
    GetWindowRect(window, &windowsize);
    OffsetRect(&desktopsize, -((windowsize.right - windowsize.left) / 2), -((windowsize.bottom - windowsize.top) / 2));
    return SetWindowPos(window, 0, desktopsize.left + (desktopsize.right - desktopsize.left) / 2, desktopsize.top + (desktopsize.bottom - desktopsize.top) / 2, 0, 0, SWP_NOSIZE);
}

//#ifdef _DEBUG
void debuglog(const char* fmt, ...)
{
    static FILE* fh = 0;
    if (!fh) {
        CreateDirectory(L"logs", 0);
        fh = fopen("logs/bf42plus_debug.log", "w");
        if (!fh) return;
    }
    va_list va;
    va_start(va, fmt);
    vfprintf(fh, fmt, va);
    va_end(va);
    fflush(fh);

}
//#endif