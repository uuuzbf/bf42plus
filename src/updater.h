#pragma once


/// <summary>
/// Get the release channel the updater updates to
/// </summary>
/// <returns></returns>
const wchar_t* get_update_release_channel();

/// <summary>
/// Get the release channel this binary is built for
/// Is this function needed?
/// </summary>
/// <returns></returns>
const wchar_t* get_build_release_channel();

/// <summary>
/// Get the binary's version
/// </summary>
/// <returns></returns>
const wchar_t* get_build_version();

/// <summary>
/// Starts update checking process when the module is loaded
/// </summary>
void updater_client_startup();
