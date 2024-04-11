## Battlefield 1942 client patches
This mod provides several improvements to Battlefield 1942.

### Installation
Copy `dsound.dll` into your game directory.

If you already have a modded `dsound.dll` in your game directory, rename it to `dsound_next.dll`, it will keep working!

### Configuration
If you want to enable some optional features, take a look at `bf42plus.ini` in the game directory. This file is automatically created if it doesn't exist.

### Updater
The DLL has a builtin updater, so it can replace itself with newer versions. When there is an update available, a window will appear on startup, with the details of the new update. You have the option to postpone updating or apply the update.

### Bugfixes
- Fix crash when the game is minimized
- Fix ping display in scoreboard
- Fix other windows getting resized when the game is launched
- Fix maplist sometimes empty
- Fix some servers greyed out in the server list
- Center kill message now shows who teamkilled you
- Game startup sped up by about a second, dependant on system.
- Fix a loading screen crash caused by a bug in the game's memory allocator

### Enhancements
- Speed up reconnecting to servers on map change
- Changing mod when joining a server from the serverlist is faster
- Optional feature: message when someone connects to the server
- Optional feature: show player IDs in chat and/or in kill messages
- Optional feature: show player IDs in nametags
- Optional feature: show who started a vote or voted
