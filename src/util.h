#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>

class HTTPClient
{
	HINTERNET session;
	HINTERNET connect;
	HINTERNET request;
	bool isHTTPS;
	DWORD secureFailFlags;
	DWORD lastError;

public:
	HTTPClient() : session(0), connect(0), request(0), isHTTPS(false), secureFailFlags(0), lastError(0) {};
	~HTTPClient();
	bool Init(const std::wstring hostname, unsigned short port = 0, bool https = false, int timeout = 0, std::optional<std::wstring> agent = std::nullopt);
	void reset();
	bool GET(const std::wstring path);
	int status();
	bool data(std::vector<char>& data);
	bool text(std::wstring& text);
	DWORD getLastError() const { return lastError; };
private:
	static VOID CALLBACK statusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
};

inline std::string& replaceAll(std::string& str, const std::string& oldstr, const std::string& newstr) {
	if (!oldstr.empty()) {
		for (size_t start_pos = 0; (start_pos = str.find(oldstr, start_pos)) != std::string::npos; start_pos += newstr.length()){
			str.replace(start_pos, oldstr.length(), newstr);
		}
	}
	return str;
}

inline std::wstring& replaceAll(std::wstring& str, const std::wstring& oldstr, const std::wstring& newstr) {
	if (!oldstr.empty()) {
		for (size_t start_pos = 0; (start_pos = str.find(oldstr, start_pos)) != std::wstring::npos; start_pos += newstr.length()) {
			str.replace(start_pos, oldstr.length(), newstr);
		}
	}
	return str;
}

std::wstring UTF8ToWideString(const char* s, size_t len);
// this function only converts ASCII characters
std::string	WideStringToASCII(std::wstring s);
// this function only converts ASCII characters
std::wstring ASCIIToWideString(const std::string& s);
bool HexStringToData(const std::wstring& str, std::vector<unsigned char>& data);
bool HexDataToData(const std::vector<char>& hex, size_t offset, size_t length, std::vector<unsigned char>& data);

bool CenterWindow(HWND window);

//#ifdef _DEBUG
void debuglog(const char* fmt, ...);
//#else
//#define debuglog	(void)
//#endif