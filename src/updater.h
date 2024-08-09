#pragma once

// A version number used for versioning customizations in the game protocol.
// Increment this on breaking network protocol change
const uint8_t PLUS_PROTOCOL_VERSION = 2;

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
/// Get the binary's version number's components
/// </summary>
void get_build_version_components(uint8_t& major, uint8_t& minor, uint8_t& patch, uint8_t& build);

/// <summary>
/// This function should be called right before the game creates its main window.
/// The function may wait until update checking or updating is done, the updater thread may
/// also restart the process after an update so this function may never return.
/// The function will be executing in a different thread than the rest of updater.cpp!
/// </summary>
void updater_wait_for_updating();

/// <summary>
/// Starts update checking process when the module is loaded
/// </summary>
void updater_client_startup();
