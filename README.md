# glitchGBA v1.2 (beta)
A simple tileset-based video glitch software for Nintendo Gameboy Advance, developed with libgba.

You may think of it as something similar to [**glitchNES**](http://no-carrier.com/index.php?/glitchnes/) by NO-CARRIER.
It was created with live video performing in mind so it's highly interactive and versatile allowing for a lot of tilesets to fit into the GBA rom.

Despite the real GBA hardware natively not being capable of video out, you can run this in a common emulator for PC, Raspberry Pi, the PSP, OpenDingux devices, even a Gamecube with the Gameboy Player, all of which can display on a real screen or projector.

## Examples

![1](http://www.meru.cloud/assets/06-07-2019/1.png) ![2](http://www.meru.cloud/assets/06-07-2019/2.png) 
![3](http://www.meru.cloud/assets/06-07-2019/3.png)

[Demo1](https://www.youtube.com/watch?v=6Cut0q85Z6Q)
[Demo2](https://youtu.be/0SPNZVn2vGY)

## How does it work?
**In normal mode:**
Gets some tilesets into the video memory, then proceeds to display the 4 layered GBA backgrounds after applying random tileset displacement. The palette can also be altered.
The layers can all be moved together or selected and moved independently to create a parallax scrolling effect.

**In rotation mode:**
Same thing but applied to only 2 layers which can however be rotated and zoommed.
To rotate them independently, select layer with B+Up/Down and enable parallax mode.

## Commands
- **D-Pad** : Change speedX/speedY of current layer (there is no indication of which layer is currently selected atm)
- **A**: Pause/Resume
- **B**: Enable/Disable wait for Vertical Sync interrupt
- **Start**: Alter palette
- **Select**: Swap layers movement between uniform and parallax
- **Hold Start, press A**: Alter mapping of glitched layers
- **Hold Select, press A**: Alter mapping of standard layer
- **Hold A, press B**: Stop movement
- **Hold B, press Up**: Select layer up
- **Hold B, press Down**: Select layer down
- **Hold Start, press B**: Enable/Disable current layer
- **Hold Select, press B**: Enable/Disable console mode (for VBA compatibility)
- **Select+Start**: Chaos mode (randomizes heavily)
- **L**: Switch to previous tileset (reglitch)
- **R**: Switch to next tileset (reglitch)
- **Hold L, press A**: Switch to previous tileset (don't reglitch)
- **Hold R, press A**: Switch to next tileset (don't reglitch)
- **Hold R, press Right**: Enable/Disable autoswapper (laggish but does its work)
- **A+B+Select+Start**: Reset ROM.
## Commands introduced in 1.2
- **Hold L, press R**: Switch between normal and rotation mode
- **Hold L, press A**: In rotation mode, turn current layer left.
- **Hold R, press A**: In rotation mode, turn current layer right.
- **Hold L, press B**: In rotation mode, zoom current layer in.
- **Hold R, press B**: In rotation mode, zoom current layer out.

Notice that 'console mode' is a compatibility option which is enabled by default. Apparently, the Visual Boy Advance emulator needs this option be disabled to work as intended.

## Building from source:
### Prerequisites
- Install [devKitPro](https://github.com/devkitPro/installer/releases). Select the GBA development.
- Make sure you have [Python3](https://www.python.org/downloads/) installed.
- Prepare the tilesets in a 256x256 sized **png** file and put it in the graphics folder under whichever name you like.
You can probably exceed this size vertically but be sure to keep the horizontal size 256 pixels. It's not vital but it's probably more organized. Also notice that the size of the tileset should be in multiples of 8 since that's the minimum size of a tile block.
You may theoretically put a lot of tiles but at some point the memory of the GBA will be full and some issues will arise in the compiler. At least a dozen should fit, however

### Actual building
- Run ```make genereate_headers```. This requires that you run [grit](https://github.com/devkitPro/grit). 
Grit is contained in the devkitpro distribution. Make sure that the devkitPro/tools/bin folder is in your PATH variable.
- Finally, run ```make```.

## Credits
- [Coranac](http://www.coranac.com/) for [grit](https://www.coranac.com/projects/grit/) and the [tonc](http://www.coranac.com/tonc/text/toc.htm) which taught me many things.
- [INFU_AV](https://twitter.com/Infu_av) for his precious feedback on the project.
- the DevKitPro team for libgba.
