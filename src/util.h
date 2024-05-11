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
// This function converts from wide strings to ISO 8859-1 (what the game uses)
// Unknown characters are replaced with "?"
std::string	WideStringToISO88591(std::wstring s);
// This function converts from ISO 8859-1 (what the game uses) to wide strings
std::wstring ISO88591ToWideString(const std::string& s);
bool HexStringToData(const std::wstring& str, std::vector<unsigned char>& data);
bool HexDataToData(const std::vector<char>& hex, size_t offset, size_t length, std::vector<unsigned char>& data);

bool CenterWindow(HWND window);

const uint32_t InvalidColor = 0xffffffff;
uint32_t GetColorFromString(const std::string& name);
std::string GetStringFromColor(uint32_t color);
std::string GetStringFromColors(std::vector<uint32_t>& colors);

struct StringCompareNoCase {
	bool operator()(const std::string& left, const std::string& right) const { return _stricmp(left.c_str(), right.c_str()) < 0; };
};

uint8_t crc8(uint8_t* data, size_t length);

// Reads the Windows MachineGuid from the registry, output buffer must be atleast 40 bytes
bool GetMachineGUID(BYTE* output, DWORD* size);

//#ifdef _DEBUG
void debuglog(const char* fmt, ...);
//#else
//#define debuglog	(void)
//#endif