#include "pch.h"
#include <shellapi.h>
#include <Shlwapi.h>

#include "gitversion.h"

static const wchar_t* UPDATE_SERVER = L"update.bf1942.hu";
static const unsigned short UPDATE_SERVER_PORT = 0; // default
static bool UPDATE_SERVER_HTTPS = true;
static const wchar_t* UPDATE_CHECK_PATH = L"/update/v2/latest_{CHANNEL}.txt";

static const wchar_t* MB_TITLE = L"BF42Plus updater";
static const char* MB_TITLEA = "BF42Plus updater";


static std::string updater_state = "uninitialized";
// this event is set when update checking is done and the game's main window can be created
static HANDLE event_update_check_done = 0;

static const unsigned char update_publickey_non_release[crypto_sign_PUBLICKEYBYTES] = {
    0x0C, 0x38, 0x36, 0x54, 0x07, 0xCA, 0xCB, 0xD7,
    0x6B, 0xC0, 0x04, 0x90, 0x0E, 0xCA, 0xDB, 0x8B,
    0xF6, 0x80, 0x59, 0xBD, 0xDD, 0xD6, 0x1B, 0x45,
    0xA6, 0xD8, 0xC9, 0xAC, 0xC7, 0xBD, 0x26, 0xA6
};

static const unsigned char update_publickey_release[crypto_sign_PUBLICKEYBYTES] = {
    0x96, 0x50, 0xec, 0x95, 0x7c, 0x72, 0x52, 0x98,
    0xb2, 0xad, 0x39, 0x93, 0x1a, 0x48, 0xa0, 0x04,
    0x5c, 0xb6, 0xd6, 0x97, 0xbf, 0xf8, 0xb2, 0xfd,
    0xb2, 0xbb, 0xd5, 0x93, 0xe9, 0x7c, 0xb0, 0x68
};

/// <summary>
/// Stores info about update to a single file
/// </summary>
struct UpdateFile {
    // path the file should be installed to
    std::wstring installpath;
    // path of the file on the update server
    std::wstring downloadpath;
    // path to temp file the file was downloaded to, or empty string
    std::wstring tempfilepath;
    // size of the file in bytes
    size_t size;
    // hash of the file for verification (libsodium crypto_generichash())
    std::vector<unsigned char> hash;
};

/// <summary>
/// Stores information between check_for_update and download_update functions
/// </summary>
struct UpdateInfo {
    // version of the update
    std::wstring version;
    // update description, may contain multiple lines
    std::wstring description;
    // list of files to be updated
    std::list<UpdateFile> updatedfiles;
    // sum of file sizes in updatedfiles
    size_t totalbytes;
    // release channel used
    std::wstring channel;
};

const wchar_t* get_update_release_channel()
{
#ifdef _DEBUG
    return L"debug";
#else
    return L"release";
#endif
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

extern const char _build_version_dummy[] = "_build_version_=" M_TO_STRING(GIT_VERSION);

static const wchar_t* get_user_agent()
{
    return L"BF42Plus/" M_TO_STRING(GIT_VERSION);
}

void get_build_version_components(uint8_t& major, uint8_t& minor, uint8_t& patch, uint8_t& build)
{
    uint16_t tmp;
    major = minor = patch = build = 0;
    auto ver = std::stringstream(M_TO_STRING(GIT_VERSION));
    ver >> tmp; major = (uint8_t)tmp;
    ver.get(); // skip delimiter
    ver >> tmp; minor = (uint8_t)tmp;
    ver.get(); // skip delimiter
    ver >> tmp; patch = (uint8_t)tmp;
    ver.get(); // skip delimiter
    ver >> tmp; build = (uint8_t)tmp;
}

int compare_version(std::wstring older, std::wstring newer)
{
    auto sOld = std::wistringstream(older), sNew = std::wistringstream(newer);
    for (int i = 0; i < 4; i++) {
        int oldN = 0, newN = 0;
        sOld >> oldN; sNew >> newN; // read a number from each version, default to 0
        if (oldN < newN) return 1; // newer is newer than older
        else if (oldN > newN) return -1; // newer is older
        sOld.get(); sNew.get(); // skip a delimiter character
    }
    return 0; // versions are equal
}


static wchar_t temp_dir[MAX_PATH];
static bool cleanup_temp_dir()
{
    DWORD len = wcslen(temp_dir);
    temp_dir[len + 1] = 0; // SHFileOperation needs double null terminated strings
    SHFILEOPSTRUCT fo = {
        0, // hwnd
        FO_DELETE,
        temp_dir, // pFrom
        0, // pTo, unused
        FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
        false, // fAnyOperationsAborted
        0, // hNameMappings
        L"" // title text, not used
    };
    if (SHFileOperation(&fo) != 0) {
        return false;
    }
    return true;
}
static bool init_temp_dir(std::wstring& dir)
{
    DWORD len = GetTempPath(MAX_PATH, temp_dir);
    if (MAX_PATH - len < 32) {
        updater_state = "init temp: path too long";
        debuglog("init_temp_dir: temp path too long\n");
        return false;
    }
    wcscat(temp_dir, L"bf42pupd\\");
    if (PathFileExists(temp_dir)) {
        if (!cleanup_temp_dir()) {
            updater_state = "init temp: failed to delete old directory";
            debuglog("init_temp_dir: failed to clean up old temp dir: %ls\n", temp_dir);
            return false;
        }
    }
    if (!CreateDirectory(temp_dir, 0)) {
        updater_state = "init temp: failed to create directory";
        debuglog("init_temp_dir: failed to create temp dir: %ls\n", temp_dir);
        return false;
    }
    dir = temp_dir;
    return true;
}

static bool check_for_update(UpdateInfo& info)
{
    debuglog("check for update\n");

    std::wstring latest_version_path = UPDATE_CHECK_PATH;
    info.channel = get_update_release_channel();
    replaceAll(latest_version_path, L"{CHANNEL}", info.channel);

    HTTPClient http;
 retry_request:
    if (!http.Init(UPDATE_SERVER, UPDATE_SERVER_PORT, UPDATE_SERVER_HTTPS, 1500, get_user_agent())) {
        updater_state = "check: init failed";
        return false;
    }

    if (!http.GET(latest_version_path)) {
        // if the request failed due to a tls error, retry with http instead of https
        // this shouldn't cause a security issue because the update txt file is signed
        // and all update files are verified by hash
        if (http.getLastError() == ERROR_WINHTTP_SECURE_FAILURE && UPDATE_SERVER_HTTPS) {
            debuglog("check: ERROR_WINHTTP_SECURE_FAILURE, disabling https\n");
            http.reset();
            UPDATE_SERVER_HTTPS = false;
            goto retry_request;
        }
        else {
            updater_state = "check: request failed";
            return false;
        }
    }

    int status = http.status();
    if (status != 200) {
        updater_state = std::format("check: server returned {}", status);
        debuglog("server returned %d\n", status);
        return false;
    }

    std::vector<char> response;
    if (!http.data(response)) {
        updater_state = "check: reading response failed";
        return false;
    }

    // the update info file is a text file with unix line endings (only LF)
    // the format of the file is the following:
    // <signature>                          - signature of the update file, starting from the beginning of the next line
    // <version>                            - version string of the current available update
    // <multiple lines of update description>
    // END_DESCRIPTION
    // <installpath>;<downloadpath>;<size>;<hash>  - a file to update
    //
    // an update info file may have multiple file-to-update lines
    // <installpath> is relative to the game's base directory (working directory of the current process)
    // <downloadpath> is a URI path on the update server
    // <size> is the size of the new file in bytes
    // <hash> is the hash of the file to be updated, using libsodum crypto_generichash
    // the update info file's last line must have a trailing newline character
    // the update info file must not contain trailing whitespaces

    // first line only contains a hexadecimal signature, so the position of the first newline is given
    size_t first_eol = crypto_sign_BYTES * 2;
    if (response.size() <= first_eol || response.at(first_eol) != '\n') {
        updater_state = "check: malformed response";
        debuglog("check: signature newline not found\n");
        return false;
    }

    std::vector<unsigned char> signature;
    if (!HexDataToData(response, 0, first_eol, signature)) {
        updater_state = "check: malformed signature";
        debuglog("check: malformed signature\n");
    }

    const char* data = response.data() + first_eol + 1;
    size_t datalength = response.size() - first_eol - 1;

    const unsigned char* pubkey = info.channel == L"release" ? update_publickey_release : update_publickey_non_release;

    if (crypto_sign_verify_detached(signature.data(), reinterpret_cast<const unsigned char*>(data), datalength, pubkey) < 0) {
        updater_state = "check: signature check failed";
        debuglog("check: crypto_sign_verify_detached failed\n");
        return false;
    }

    auto ss = std::wstringstream{ UTF8ToWideString(data, datalength) };
    std::getline(ss, info.version);
    debuglog("update check version: %ls\n", info.version.c_str());
    
    // version string is read, we can do the need-to-update check here
    
    // update if:
    //  updating to a different release channel OR
    //  build version differs

    // if release channels don't match then always update, otherwise check version
    if (wcscmp(get_update_release_channel(), get_build_release_channel()) == 0) {

        int compareResult = compare_version(get_build_version(), info.version);
        if (compareResult == 0) {
            updater_state = "up-to-date";
            return false;
        }
        else if(compareResult == -1){
            updater_state = "newer-than-update-server";
            return false;
        }
        // compareResult is 1, update needed, continue with parsing update info
    }

    // read description
    std::wstringstream description;
    for (std::wstring line;;) {
        if (!std::getline(ss, line)) {
            updater_state = "check: unterminated description";
            debuglog("check: unterminated description\n");
            return false;
        }
        if (line == L"END_DESCRIPTION") {
            break;
        }
        description << line << "\r\n";
    }
    info.description = description.str();
    debuglog("description:\n%ls", info.description.c_str());

    // read file update info
    for (;;)
    {
        UpdateFile file;
        if (!std::getline(ss, file.installpath, ss.widen(';'))) {
            // end of file reached
            break;
        }
        std::getline(ss, file.downloadpath, ss.widen(';'));
        ss >> file.size;
        ss.get(); // skip ; after size
        std::wstring hashstr;
        std::getline(ss, hashstr); // last field, also read EOL
        // unhex the hash, and do basic checks on other fields
        if (!HexStringToData(hashstr, file.hash) || file.hash.size() != crypto_generichash_BYTES || file.size == 0 || file.downloadpath.empty() || file.installpath.empty()) {
            updater_state = "check: parsing file failed";
            debuglog("check: parsing file failed, downloadpath:'%ls' installpath:'%ls' size:%zu hashstr:'%ls'\n", file.downloadpath.c_str(), file.installpath.c_str(), file.size, hashstr.c_str());
            return false;
        }

        info.updatedfiles.push_back(file);
    }

    return true;
}

// download updated file, verify hash and save it somewhere
static bool download_update_file(HTTPClient& http, UpdateFile& file, const std::wstring& tempdir)
{
    wchar_t tempfilepath[MAX_PATH];
    if (GetTempFileName(tempdir.c_str(), L"UPD", 0, tempfilepath) == 0) {
        updater_state = "download: get temp file failed";
        debuglog("download: failed to create temp file in %ls\n", tempdir.c_str());
        return false;
    }

    if (!http.GET(file.downloadpath)) {
        updater_state = "download: request failed";
        return false;
    }

    int status = http.status();
    if (status != 200) {
        updater_state = std::format("download: server returned {}", status);
        debuglog("download: server returned %d\n", status);
        return false;
    }

    std::vector<char> filedata;
    if (!http.data(filedata)) {
        updater_state = "download: reading data failed";
        return false;
    }

    unsigned char downloadhash[crypto_generichash_BYTES];
    // return value not documented? never fails?
    crypto_generichash(downloadhash, sizeof(downloadhash), reinterpret_cast<const unsigned char*>(filedata.data()), filedata.size(), 0, 0);

    if (file.hash.size() != crypto_generichash_BYTES || memcmp(downloadhash, file.hash.data(), sizeof(downloadhash)) != 0) {
        updater_state = "download: file verification failed";
        debuglog("download: hash of file %ls is incorrect\n", file.downloadpath.c_str());
        return false;
    }

    {
        std::ofstream fs(tempfilepath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (fs.fail()) {
            updater_state = "download: failed to create file";
            debuglog("failed to create file when updating: %ls\n", tempfilepath);
            return false;
        }

        fs.write(filedata.data(), filedata.size());
        if (fs.bad()) {
            updater_state = "download: writing file data failed";
            debuglog("failed to write file when updating: %ls\n", tempfilepath);
            return false;
        }
    }

    file.tempfilepath = tempfilepath;

    debuglog("downloaded %ls to %ls\n", file.downloadpath.c_str(), file.tempfilepath.c_str());
    return true;
}


static bool download_update(UpdateInfo& info, const std::wstring& tempdir)
{
    HTTPClient http;
    if (!http.Init(UPDATE_SERVER, UPDATE_SERVER_PORT, UPDATE_SERVER_HTTPS, 5000)) {
        updater_state = "download: http init failed";
        return false;
    }

    for (auto& file : info.updatedfiles) {
        if (!download_update_file(http, file, tempdir)) {
            MessageBox(0, L"Failed to download an update file", MB_TITLE, MB_ICONERROR);
            debuglog("failed to download file %ls\n", file.downloadpath.c_str());
            return false;
        }
    }
    return true;
}

// replace loaded dll with new one
static bool apply_update(UpdateInfo& info)
{
    for (auto& file : info.updatedfiles) {
        std::wstring oldfilepath = file.installpath + L".oldversion";
        // try deleting <targetfile>.oldversion in every case to clean up
        if (DeleteFile(oldfilepath.c_str())) {
            debuglog("apply: old %ls deleted\n", oldfilepath.c_str());
        }

        bool oldFileMoved = false;
        // if target file exists, add .oldversion suffix
        if (PathFileExists(file.installpath.c_str())) {
            if (!MoveFile(file.installpath.c_str(), oldfilepath.c_str())) {
                debuglog("apply: failed to move %ls to %ls\n", file.installpath.c_str(), oldfilepath.c_str());
                updater_state = "apply: failed to move file to .oldversion";
                return false;
            }
            oldFileMoved = true;
        }

        // move the new file to the target path
        if (!MoveFile(file.tempfilepath.c_str(), file.installpath.c_str())) {
            debuglog("apply: failed to move temp %ls to %ls\n", file.tempfilepath.c_str(), file.installpath.c_str());
            updater_state = "apply: failed to move new file from temp";
            // try moving back the original file
            if (oldFileMoved && !MoveFile(oldfilepath.c_str(), file.installpath.c_str())) {
                debuglog("apply: failed to recover old file\n");
            }
            return false;
        }
        // file update applied, clear temp path
        file.tempfilepath.clear();
        debuglog("applied update %ls\n", file.installpath.c_str());
    }
    return true;
}

BOOL CALLBACK UpdateAvailableProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            CenterWindow(hwnd);

            UpdateInfo* info = reinterpret_cast<UpdateInfo*>(lParam);
            SetWindowText(GetDlgItem(hwnd, IDC_STATIC_OLD_VERSION), get_build_version());
            SetWindowText(GetDlgItem(hwnd, IDC_STATIC_NEW_VERSION), info->version.c_str());
            SetWindowText(GetDlgItem(hwnd, IDC_UPDATE_DESCRIPTION), info->description.c_str());
            HWND filelist = GetDlgItem(hwnd, IDC_UPDATE_FILES);
            LVCOLUMN col;
            col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
            col.fmt = LVCFMT_LEFT;
            col.iSubItem = 0;
            col.pszText = const_cast<wchar_t*>(L"File");
            col.cx = 250;
            ListView_InsertColumn(filelist, col.iSubItem, &col);
            col.iSubItem = 1;
            col.pszText = const_cast<wchar_t*>(L"Size");
            col.cx = 80;
            ListView_InsertColumn(filelist, col.iSubItem, &col);

            int row = 0;
            for (auto it = info->updatedfiles.begin(); it != info->updatedfiles.end(); it++)
            {
                LVITEM item;
                item.mask = LVIF_TEXT;
                item.iItem = row++;
                item.iSubItem = 0;
                item.pszText = const_cast<wchar_t*>(it->installpath.c_str());
                ListView_InsertItem(filelist, &item);
                item.iSubItem = 1;
                wchar_t size[32];
                _snwprintf(size, 32, L"%u", it->size);
                item.pszText = size;
                ListView_SetItem(filelist, &item);
            }
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: // update button pressed
                case IDCANCEL: // skip button
                    EndDialog(hwnd, wParam); // DialogBoxParam() will return either IDOK or IDCANCEL
                    return TRUE;
            }

    }
    return FALSE;
}

/// <summary>
/// restart bf1942.exe with same parameters
/// </summary>
void restart_bf1942()
{
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    if (!CreateProcess(0, GetCommandLine(), 0, 0, false, CREATE_NEW_PROCESS_GROUP, 0, 0, &si, &pi)) {
        char errmsg[256];
        _snprintf(errmsg, 256, "Failed to create process when restarting BF1942.exe\n\nError code: %08X", GetLastError());
        errmsg[255] = 0;
        MessageBoxA(0, errmsg, MB_TITLEA, MB_ICONERROR);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    ExitProcess(0);
}

void updater_wait_for_updating()
{
    if (event_update_check_done == 0) return;
    WaitForSingleObject(event_update_check_done, INFINITE);
}

static DWORD __stdcall updater_thread(void* ptr)
{
    (void)ptr;
    debuglog("updater thread started\n");
    if (sodium_init() < 0) {
        updater_state = "sodium_init failed";
        debuglog("sodium_init failed\n");
        return 1;
    }
    UpdateInfo info;
    if (check_for_update(info)) {
        int result = DialogBoxParam((HINSTANCE)g_this_module, MAKEINTRESOURCE(IDD_UPDATE_AVAILABLE), 0, UpdateAvailableProc, reinterpret_cast<LPARAM>(&info));
        debuglog("DialogBoxParam = %d  GetLastError() = %u\n", result, GetLastError());

        if (result == IDOK) {
            std::wstring tempdir;
            if (init_temp_dir(tempdir)) {
                if (download_update(info, tempdir)) {
                    if (apply_update(info)) {
                        updater_state = "update applied";
                        MessageBox(0, L"Updating complete!\n\nBattlefield 1942 will now restart!", MB_TITLE, 0);
                        restart_bf1942();
                    }
                    else {
                        MessageBox(0, L"Applying update failed, continuing with old version.", MB_TITLE, MB_ICONEXCLAMATION);
                    }
                }
                else {
                    MessageBox(0, L"Downloading update failed, continuing with old version.", MB_TITLE, MB_ICONEXCLAMATION);
                }
                cleanup_temp_dir();
            }
            else {
                MessageBox(0, L"Failed to create temporary directory for downloading, continuing with old version.", MB_TITLE, MB_ICONEXCLAMATION);
            }
        }
        else {
            updater_state = "update skipped by user";
        }
    }
#ifdef _DEBUG
    if (MessageBoxA(0, updater_state.c_str(), "start game?", MB_ICONINFORMATION | MB_YESNO) == IDNO) {
        ExitProcess(0);
    }
#endif
    SetEvent(event_update_check_done);

    // write status string to file, this is a temporary solution until some user interface will be available
    std::ofstream status("updatestatus.txt", std::ios::out | std::ios::trunc);
    std::time_t now = std::time(0);
    char now_str[64];
    std::strftime(now_str, 64, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    status << now_str << " " << updater_state;
    return 0;
}

// called from mainhook
// start a thread to do updating in background
void updater_client_startup()
{
    event_update_check_done = CreateEvent(0, TRUE, FALSE, 0);
    CreateThread(0, 0, updater_thread, 0, 0, 0);
}
