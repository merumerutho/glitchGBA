# glitchGBA v1.0
A simple tileset-based video glitch software for Nintendo Gameboy Advance, developed with libgba.

You may think of it as something similar to **glitchNES** by NO-CARRIER.
It was created with live video performing in mind so it's highly interactive and versatile allowing for a lot of tilesets to fit into the GBA rom.

Despite the real GBA hardware natively is not capable of video out, you can run this in a common emulator (it's easy and free, GBA emulators also runs on outdated hardware!) for PC, SBCs like the Raspberry Pi, the PSP, the RS-97 or similar, or even a Gamecube with the Gameboy Player, all of which can display on a real screen or projector.

## Examples

![1](https://www.meru.cloud/assets/glitchGBA/1.PNG) ![2](https://www.meru.cloud/assets/glitchGBA/2.PNG) 

![3](https://www.meru.cloud/assets/glitchGBA/3.PNG) ![4](https://www.meru.cloud/assets/glitchGBA/4.PNG)

[Demo1](https://www.youtube.com/watch?v=6Cut0q85Z6Q)
[Demo2](https://youtu.be/0SPNZVn2vGY)

## Example release:
[Download here](https://www.meru.cloud/assets/glitchGBA/glitchGBA.gba)

## How does it work?
It just copies some tilesets into the video memory, then proceeds to display it (and move it) layering the 4 GBA backgrounds as a single non-glitched layer and three glitched layers. It can alter the palette as well.
Select a layer and alter it, have fun!

## Commands
- **D-Pad** : Change speedX/speedY of current layer (there is no indication of which layer is currently selected atm)
- **A**: Pause/Resume
- **B**: Enable/Disable wait for Vertical Sync interrupt
- **Start**: Alter palette
- **Select**: Swap layers movement between uniform and parallax
- **A+Start**: Alter mapping of glitched layers
- **A+Select**: Alter mapping of standard layer
- **A+B**: Stop movement
- **B+Up**: Select layer up
- **B+Down**: Select layer down
- **B+Start**: Enable/Disable current layer
- **B+Select**: Enable/Disable console mode
- **Select+Start**: Chaos mode (randomizes heavily)
- **L**: Switch to previous tileset (reglitch)
- **R**: Switch to next tileset (reglitch)
- **L+A**: Switch to previous tileset (don't reglitch)
- **R+A**: Switch to next tileset (don't reglitch)
- **R+Right**: Enable/Disable autoswapper (laggish but does its work)

Notice that console mode is a mode for running on real hardware (and some emulators, like the one on RS-97).
There is a mismatch in the way Visual Boy Advance and the real hardware interpret the movement of the tilesets.
Selecting console mode allows for the correct experience on console.

## How to compile the ROM with your own graphics:
### Windows:
- Install [devKitPro](https://github.com/devkitPro/installer/releases). Select the GBA development.
- Clone the git or download the zip and extract it, then open the folder.
- Prepare the tilesets in a 256x256 sized **png** file and put it in the graphics folder under whichever name you like.
You can probably exceed this size vertically but be sure to keep the horizontal size 256 pixels. It's not vital but it's probably more organized. Also notice that the size of the tileset should be in multiples of 8 since that's the minimum size of a tile block.
You may theoretically put a lot of tiles but at some point the memory of the GBA will be full and some issues will arise in the compiler. At least a dozen should fit, however
- Once you're ready, go back to the main folder and launch **compile.exe**. The rom will compile.

## Credits
- [TurboGamma](https://www.facebook.com/turbogamma/) for one of the included graphics.
- [Coranac](http://www.coranac.com/) for [grit](https://www.coranac.com/projects/grit/) and the [tonc](http://www.coranac.com/tonc/text/toc.htm) which taught me many things.
- the DevKitPro team for libgba.

