## Battlefield 1942 client patches
This mod provides several improvements to Battlefield 1942.

**IMPORTANT: the mod has been changed to use `dsound.dll` instead of `d3d8.dll`. To update manually remove the old `d3d8.dll` and download the new `dsound.dll`, then place that in your game directory.**

### Installation
Copy `dsound.dll` into your game directory.

If you already have a modded `dsound.dll` in your game directory, rename it to `dsound_next.dll`, it will keep working!

### Updater
The DLL has a builtin updater, so it can replace itself with newer versions. For now the updater cannot be disabled but this option will be added when configuration will be possible.

### Bugfixes
- Fix crash when the game is minimized
- Fix ping display in scoreboard
- Fix other windows getting resized when the game is launched
- Fix maplist sometimes empty
- Fix some servers greyed out in the server list

### Enhancements
- Speed up reconnecting to servers on map change
- Changing mod when joining a server from the serverlist is faster
