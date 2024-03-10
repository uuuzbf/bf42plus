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

public:
	HTTPClient() : session(0), connect(0), request(0), isHTTPS(false) {};
	~HTTPClient();
	bool Init(const std::wstring hostname, unsigned short port = 0, bool https = false);
	bool GET(const std::wstring path);
	int status();
	bool data(std::vector<char>& data);
	bool text(std::wstring& text);
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
std::string	WideStringToASCII(std::wstring s);
bool HexStringToData(const std::wstring& str, std::vector<unsigned char>& data);

#ifdef _DEBUG
void debuglog(const char* fmt, ...);
#else
#define debuglog (void)
#endif