## Battlefield 1942 client patches
This mod provides several [fixes](#bugfixes) and [improvements](#improvements) to Battlefield 1942. It also has some [additional features](#optional-features) however these are all disabled by default. It is also possible to [set custom colors](#buddy-colors) for players on the buddy list.

The current version is `1.3.3`. You can see your version in the main menu in the bottom left corner.

### Installation
Download the [latest release](https://github.com/uuuzbf/bf42plus/releases) and copy `dsound.dll` into your game directory.

_If you already have a modded `dsound.dll` in your game directory, rename it to `dsound_next.dll`, it will keep working!_

#### Missing MSVCP140.dll error
If you get an error about missing MSVCP140.dll, install the following package from Microsoft:
https://aka.ms/vs/17/release/vc_redist.x86.exe
**Note that you have to install the 32 bit (x86) package on 64 bit Windows too!**

#### Extra steps under Wine
The mod works under Wine, but by default libraries placed next to executables aren't loaded. To fix this you have to run `winecfg`. On the `Applications` tab add `BF1942.exe`, then on the `Libraries` tab add `dsound` to the dll overrides.

### Support
If you have any issues with the mod or want to ask for bugfixes or features then you can do it here by opening an issue, in the [SiMPLE forum topic](https://team-simple.org/forum/viewtopic.php?id=10835) or on Discord by messaging me (uuuzbf).

### Configuration
If you want to enable some optional features, take a look at `bf42plus.ini` in the game directory, it has explanations for each available setting. This file is automatically created after first launch.

Alternatively you can change the configuration from the game's ingame console, type `plus.` then tap TAB twice to see the available commands.

### Updater
The DLL has a builtin updater, so it can replace itself with newer versions. When there is an update available, a window will appear on startup, with the details of the new update. You have the option to postpone updating or apply the update. The mod only sends the mod's version string to the update server when checking for updates. No other data is transmitted. There is currently no option to disable update checking because the mod is still being actively developed and having people running outdated versions would defeat the purpose of the mod. All new future features/fixes that have any effect on the game will either be disabled by default or there will be the option to turn it off. Updates are cryptographically signed so if the update server is compromised its still not possible for an attacker to push out a malicious update, the update functionality will just be broken until the update server is restored. The update code is also designed to fail silently if it can't reach the update server.

### Buddy colors
You can now assign different colors to buddies by clicking the `ADD BUDDY` button on the scoreboard repeatedly. It cycles between 4 different colors, or if the SHIFT key is pressed it assigns random colors.

Another way to assign colors to buddies is to use the `plus.buddyColor`  command. You can specify colors in `#RRGGBB`, `#RGB` formats or a [webcolor name](https://www.w3schools.com/colors/colors_names.asp).

Some examples:
- Set player _chicken_ to pink: `plus.buddycolor chicken pink`
- After adding yourself as a buddy you can set your own color too: `plus.buddycolor yournick green`
- Don't forget to use quotes if the nickname has spaces: `plus.buddycolor "name with spaces" #ADD8E6`


You can also change the buddy colors in `bf42plus.ini` in the `[buddycolors]` section.

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
- Nametags are lowered when u get close to other players so you can see it. Can be disabled by adjusting `lowerNametags`
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
- Chat/kill logging to file

### Details about `plus.smootherGameplay` option
In version `1.3.4` a new option was added that may make the game run smoother and reduce the time between you pressing a key and the server processing it. The impact of this is far less than the server-side "reg patch" but it may improve your game experience a bit. It is also possible that it won't have any effect on your game.

This option is really two adjustments under the hood. The first one tells DirectX to not mess with the CPU's floating point precision, so some calculations will be more accurate. The second one tries to clear a buffer on the server belonging to you, every time you die.

By default the DirectX code changes the FPU to single precision when a Direct3D Device is created. This affects the precision of some calculations in the game, mainly the ones responsible for timing the game's frames and input updates. Normally the game runs at 60 FPS, this means a game tick runs each 16.6666 milliseconds (0.01666... seconds). There is also a global precise timer counting the number of seconds since the game started. When the game calculates when the next frame processing should start, it adds the target frame interval (0.016666...) to the time the current frame was started. For example if the current frame was started at `232.481s` the next one should start at `232.4976s`. However if the FPU is set to single precision by DirectX, this calculation is less accurate, and each frame may run a bit early or late, which depends on the actual value of the global timer. If its value is below 2048 seconds (~34 minutes, this value may be familiar to some of you), it gets rounded down and you get slightly higher framerate, if the value is between 2048 and 4096 seconds (34min - 68min), it gets rounded up, which means the framerate is a little lower. The error is about 1%, so you get/lose an extra frame roughly each 100 seconds. This not only affects the game's frame rate, but also the input timing of the game. Normally the game should read your inputs exactly 30 times a second, but this is also happening either a little bit slower or faster. 

Running the input code at a faster rate has the effect of your client sending inputs to the server faster than it gets processed by it. The server has a queue (buffer) for each player where it stores the inputs it receives before using it. When it receives a new input it adds it on one end of this queue, and when the server needs an input for a frame simulation it removes one from the other end. This queue by default has a hard limit of 4 items. If there is more, the server starts dropping the oldest ones until there is only four in it. This is actually noticeable if you know what to look for because the inputs are directly tied to the game simulation, and if one is dropped the client will adjust its worldTime by one input frame (33ms). This usually results in a small jitter, which is most noticeable if you are moving the mouse while aiming because the camera jumps back a little bit. Each item in this queue means the server will process your input 33ms later, if it has 4 items, that is a total of 133ms, which is not much but sometimes noticeable, especially if you have low ping, because then this delay is more unusual. This delay mainly shows up in events reported to the client, for example you pressing the fire button and the server sending you the kill message that your target died. The clientside lag compensation code calculates with this extra delay, however I haven't done much research on the lag compensation code yet, so this part is not clear to me. 

Servers don't have Direct3D, which means they already run the same frame timing code with this increased precision. Changing the floating point precision results in more accurate client frame timing, which means it will match the server more closely. This may have other benefits too by matching the server better when doing other calculations during game simulation, however I haven't researched this at all yet, also most of the simulation code uses single precision floats which shouldn't be affected by this change. 

The other thing this option does is trying to empty your server side input queue on each death, by dropping three inputs instead of sending it to the server. Which either drops 3 items from the queue if its full (has 4 items) or completely empties it. If it gets completely empty, your game will glitch out for a bit until it gets a new item, but because you just died, it doesn't really matter, also the effects should be mostly hidden by the death camera flying up in the air.

The performance impact of running in higher precision should be very small because on modern systems the game's CPU usage mostly consists of busy looping until the next frame. Higher precision mode only affects certain floating point operations (`fdiv`, `fsqrt` - division and square root calculations)

All this won't make you a pro player, but may get rid of some annoyances of this game. Also keep it in mind that not every system may have those issues. Some people even saw differences in the "smoothness" of the game, some didn't. This may even be affected by things like if you are running in window mode or not. However it shouldn't have any negative effects, so there is no reason to not enable it.

