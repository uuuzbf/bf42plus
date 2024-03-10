#include "pch.h"

static const wchar_t* UPDATE_SERVER = L"update.bf1942.hu";
static const unsigned short UPDATE_SERVER_PORT = 0; // default
static const bool UPDATE_SERVER_HTTPS = true;
static const wchar_t* UPDATE_CHECK_PATH = L"/update/latest_{CHANNEL}.txt";

static const unsigned char update_publickey_non_release[crypto_sign_PUBLICKEYBYTES] = {
    0xBA, 0xB4, 0x90, 0x7C, 0x70, 0x8C, 0xFF, 0xCF, 0x2C, 0xC1, 0x36, 0x14,
    0x4D, 0xDC, 0x78, 0x65, 0xF0, 0xF4, 0xFD, 0x09, 0x1E, 0x16, 0x09, 0xD9,
    0xB5, 0xFE, 0xCA, 0x95, 0xE7, 0x07, 0xEE, 0x99
};

static const unsigned char update_publickey_release[crypto_sign_PUBLICKEYBYTES] = {
    0x96, 0x50, 0xec, 0x95, 0x7c, 0x72, 0x52, 0x98,
    0xb2, 0xad, 0x39, 0x93, 0x1a, 0x48, 0xa0, 0x04,
    0x5c, 0xb6, 0xd6, 0x97, 0xbf, 0xf8, 0xb2, 0xfd,
    0xb2, 0xbb, 0xd5, 0x93, 0xe9, 0x7c, 0xb0, 0x68
};

/// <summary>
/// Stores information between check_for_update and download_update functions
/// </summary>
struct UpdateInfo {
    // path of update file on the update server
    std::wstring updatefile;
    // signature bytes
    std::vector<unsigned char> signature;
    // release channel used
    std::wstring channel;
};

const wchar_t* get_update_release_channel()
{
    return L"release";
}

const wchar_t* get_build_release_channel()
{
#ifdef _DEBUG
    return L"debug";
#else
    return L"release";
#endif
}


#define M_TO_STRING_(s)	#s
#define M_TO_STRING(s)	M_TO_STRING_(s)

const wchar_t* get_build_version()
{
    return L"" M_TO_STRING(GIT_VERSION);
}

static bool check_for_update(UpdateInfo& info)
{
    debuglog("check for update\n");
    HTTPClient http;
    if (!http.Init(UPDATE_SERVER, UPDATE_SERVER_PORT, UPDATE_SERVER_HTTPS)) {
        return false;
    }

    std::wstring latest_version_path = UPDATE_CHECK_PATH;
    replaceAll(latest_version_path, L"{CHANNEL}", get_update_release_channel());
    if (!http.GET(latest_version_path)) {
        return false;
    }

    int status = http.status();
    if (status != 200) {
        debuglog("server returned %d\n", status);
        return false;
    }

    std::wstring response;
    if (!http.text(response)) {
        return false;
    }

    auto ss = std::wstringstream{ response };
    std::wstring version, hexsignature;
    std::getline(ss, version);
    std::getline(ss, hexsignature);
    std::getline(ss, info.updatefile);

    debuglog("newest version: '%ls'\nsignature: '%ls'\nupdatefile: '%ls'\n", version.c_str(), hexsignature.c_str(), info.updatefile.c_str());

    if (hexsignature.size() != crypto_sign_BYTES*2 || info.updatefile.size() == 0) {
        return false;
    }

    if (!HexStringToData(hexsignature, info.signature)) {
        debuglog("invalid update signature\n");
        return false;
    }

    // update if:
    //  updating to a different release channel OR
    //  build version differs
    return wcscmp(get_update_release_channel(), get_build_release_channel()) != 0 || version != get_build_version();
}

// download updated file, verify it with pubkey and save it somewhere
static bool download_update(UpdateInfo& info)
{
    wchar_t modulepath[MAX_PATH];
    DWORD result = GetModuleFileName(g_this_module, modulepath, MAX_PATH);
    if (result > (MAX_PATH - 10)) {
        debuglog("module path is too long!\n");
        return false;
    }

    wcscat(modulepath, L".updated");

    std::ofstream updatefs(modulepath, std::ios::out | std::ios::trunc | std::ios::binary);

    if (updatefs.fail()) {
        debuglog("failed to create new file when updating: %ls\n", modulepath);
        return false;
    }

    HTTPClient http;
    if (!http.Init(UPDATE_SERVER, UPDATE_SERVER_PORT, UPDATE_SERVER_HTTPS)) {
        return false;
    }

    if (!http.GET(info.updatefile)) {
        return false;
    }

    int status = http.status();
    if (status != 200) {
        debuglog("server returned %d\n", status);
        return false;
    }

    std::vector<char> filedata;
    if (!http.data(filedata)) {
        return false;
    }

    if (sodium_init() < 0) {
        debuglog("sodium_init failed\n");
        return false;
    }

    const unsigned char* pubkey = info.channel == L"release" ? update_publickey_release : update_publickey_non_release;

    if (crypto_sign_verify_detached(info.signature.data(), reinterpret_cast<unsigned char*>(filedata.data()), filedata.size(), pubkey) < 0) {
        debuglog("crypto_sign_verify_detached failed\n");
        return false;
    }

    updatefs.write(filedata.data(), filedata.size());
    if (updatefs.bad()) {
        return false;
    }

    debuglog("downloaded update to %ls\n", modulepath);

    return true;
}

// replace loaded dll with new one
static bool apply_update()
{
    // renaming works even when the dll is currently loaded
    std::wstring modulepath;
    modulepath.resize(MAX_PATH);
    DWORD result = GetModuleFileName(g_this_module, modulepath.data(), MAX_PATH);
    if (result > (MAX_PATH - 10)) {
        debuglog("module path is too long!\n");
        return false;
    }
    modulepath.resize(result);

    auto updated_path = modulepath + L".updated";
    auto old_path = modulepath + L".oldversion";

    if (DeleteFile(old_path.c_str())) {
        debuglog("old update file deleted\n");
    }

    if (!MoveFile(modulepath.c_str(), old_path.c_str())) {
        debuglog("failed to move dll from %ls to %ls\n", modulepath.c_str(), old_path.c_str());
        return false;
    }

    if (!MoveFile(updated_path.c_str(), modulepath.c_str())) {
        debuglog("failed to move dll from %ls to %ls\n", updated_path.c_str(), modulepath.c_str());
        return false;
    }
    debuglog("update applied");
    return true;
}

static DWORD __stdcall updater_thread(void* ptr)
{
    debuglog("updater thread started\n");
    UpdateInfo info;
    if (check_for_update(info)) {
        if (download_update(info)) {
            apply_update();
        }
    }
    return 0;
}

// called from mainhook
// start a thread to do updating in background
void updater_client_startup()
{
    CreateThread(0, 0, updater_thread, 0, 0, 0);
}