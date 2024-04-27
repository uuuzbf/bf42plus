## Battlefield 1942 client patches
This mod provides several [fixes](#bugfixes) and [improvements](#improvements) to Battlefield 1942. It also has some [additional features](#optional-features) however these are all disabled by default.

The current version is `1.3.0`. You can see your version in the main menu in the bottom left corner.

### Installation
Copy `dsound.dll` into your game directory.

_If you already have a modded `dsound.dll` in your game directory, rename it to `dsound_next.dll`, it will keep working!_

#### Support
If you have any issues with the mod or want to ask for bugfixes or features then you can do it here by opening an issue, in the [SiMPLE forum topic](https://team-simple.org/forum/viewtopic.php?id=10835) or on Discord by messaging me (uuuzbf).

#### Extra steps under Wine
The mod works under Wine, but by default libraries placed next to executables aren't loaded. To fix this you have to run `winecfg`. On the `Applications` tab add `BF1942.exe`, then on the `Libraries` tab add `dsound` to the dll overrides.

### Configuration
If you want to enable some optional features, take a look at `bf42plus.ini` in the game directory, it explanations for each available setting. This file is automatically created after first launch.

Alternatively you can change the configuration from the game's ingame console, type `plus.` then tap TAB twice to see the available commands.

### Updater
The DLL has a builtin updater, so it can replace itself with newer versions. When there is an update available, a window will appear on startup, with the details of the new update. You have the option to postpone updating or apply the update. The mod only sends the mod's version string to the update server when checking for updates. No other data is transmitted.

### Buddy colors
You can now assign different colors to buddies by clicking the `ADD BUDDY` button on the scoreboard repeatedly. It cycles between 4 different colors, or if the SHIFT key is pressed it assigns random colors.

### Bugfixes
- Fix crash when the game is minimized
- Fix ping display in scoreboard
- Fix other windows getting resized when the game is launched
- Fix maplist sometimes empty
- Fix some servers greyed out in the server list
- Center kill message now shows who teamkilled you
- Game startup sped up by about a second, dependant on system
- Fix a loading screen crash caused by a bug in the game's memory allocator
- Fix dead players showing as alive snipers after connecting to a server
- Fix broken playernames breaking the middle kill message

### Improvements
- Speed up reconnecting to servers on map change
- Changing mod when joining a server from the serverlist is faster
- `game.showFPS`, `game.showStats`, etc screens are adjusted if you are using a different sized font, the font color is now yellow for better readability and can be changed with `debugTextColor` setting
- Yellow server messages are immediatedly copied to the console so one isn't lost if it is overwritten by another

### Optional features
These features are disabled by default. Edit `bf42plus.ini` in the game directory to enable them.
- Message when someone connects to the server (option `showConnectsInChat`)
- Show player IDs in chat and/or in kill messages (option `showIDInChat` and `showIDInKills`)
- Show player IDs in nametags (option `showIDInNametags`)
- Show in the console who started a vote or voted (option `showVoteInConsole`)
- You can start the game temporarily in windowed mode if u hold SHIFT while the game is starting and select Yes

### Planned features
- A 3D map like the one in BF:Vietnam
- Settings GUI for changing options
- Extending the game protocol to allow servers to create any static object
- Colors in console
- Better screenshots: timestamp in filename, png or jpg files, different save location(?)
- Customizing ID display in chat and nametags
- Improve windowed mode

### Experiment
Version 1.3.0 has a few optional (disabled by default!) patches, which may improve overall gameplay experience, however it needs more testing. When it is enabled the game applies the patches with a 50% chance on startup. When a multiplayer map ends it is revealed wether it was enabled or not via a message in the chat and on the middle death message. This way one can form an opinion about gameplay experience before knowing if the patches are active or not.

To enable this blind test mode with the patches, change the setting `highPrecBlindTest` in the config to `on` (or `plus.highPrecBlindTest 1` in the console, then restart). This config option will be removed in the next version when the test ends. The patches may become default enabled or disabled depending on how the tests go.

Please report your experiences in the [topic](https://team-simple.org/forum/viewtopic.php?id=10835) on the SiMPLE forum or to me (uuuzbf on Discord) directly, or feel free to open a GitHub issue about it.
