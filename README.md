# glitchGBA
A simple tileset-based video glitch software for Nintendo Gameboy Advance, developed with libgba.

You may think of it as something similar to **glitchNES** by NO-CARRIER.
It was created with live video performing in mind so it's highly interactive and versatile allowing for a lot of tilesets to fit into the GBA rom.

Despite the real GBA hardware natively is not capable of video out, you can run this in a common emulator (it's easy and free, GBA emulators also runs on outdated hardware!) for PC, SBCs like the Raspberry Pi, the PSP, the RS-97 or similar, or even a Gamecube with the Gameboy Player, all of which can display on a real screen or projector.

## Video Example
[Watch on Youtube](https://www.youtube.com/watch?v=6Cut0q85Z6Q)

## Alpha release:
[Download here](https://www.meru.cloud/assets/glitchGBA/glitchGBA-alpha-0.1.gba)

## How does it work?
It just copies some tilesets into the video memory, then proceeds to display it (and move it) layering the 4 GBA backgrounds as a single non-glitched layer and three glitched layers. It can alter the palette as well.
Select a layer and alter it, have fun!

## Commands
- **D-Pad** : change speedX/speedY of current layer (there is no indication of which layer is currently selected atm)
- **A**: pause/resume
- **B**: enable/disable wait for Vertical Sync interrupt
- **Start**: Alter palette
- **Select**: Swap layers movement between uniform and parallax
- **A+Start**: Alter mapping of glitched layers
- **A+Select**: Alter mapping of standard layer
- **A+B**: Stop movement
- **B+Up**: Select layer up
- **B+Down**: Select layer down
- **B+Start**: Enable/Disable current layer
- **B+Select**: Enable/Disable console mode

Notice that console mode is a mode for running on real hardware (and some emulators, like the one on RS-97).
There is a mismatch in the way Visual Boy Advance and the real hardware interpret the movement of the tilesets.
Selecting console mode allows for the correct experience on console.

Use for **L** and **R** keys is still reserved for future implementation of tileset switching.

## Compile instructions
You need the libgba library and the other tools included in [devKitPro](https://github.com/devkitPro/installer/releases) to compile.

Use [Usenti](https://www.coranac.com/projects/usenti/) to prepare the tileset.
Load your graphics into usenti, export creating also a map calling the created files tiles1.c/h.
Then copy into the source folder. Compile, and it's done.

## Todo
- Tileset switching
- Easier compiling chain to let non-programmers access this project comfortably.
