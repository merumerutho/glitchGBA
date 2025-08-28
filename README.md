# glitchGBA

A simple tileset-based video glitch software for Nintendo Gameboy Advance, developed with libgba.

You may think of it as something similar to [**glitchNES**](http://no-carrier.com/index.php?/glitchnes/) by NO-CARRIER. It was created with live video performing in mind so it's highly interactive and versatile allowing for a lot of tilesets to fit into the GBA rom.

Despite the fact that the original GBA hardware natively is not capable of producing external video output, you can run this in a common emulator for PC, Raspberry Pi, the PSP, modern handheld devices, even a Gamecube with the Gameboy Player, all of which can display on a real screen or projector.

## Examples

![1](http://www.meru.cloud/assets/06-07-2019/1.png) ![2](http://www.meru.cloud/assets/06-07-2019/2.png) 
![3](http://www.meru.cloud/assets/06-07-2019/3.png)

[Demo1](https://www.youtube.com/watch?v=6Cut0q85Z6Q)
[Demo2](https://youtu.be/0SPNZVn2vGY)

## How It Works

GlitchGBA now features three distinct modes that can be cycled through with **L + R**:

### Normal Mode
Gets some tilesets into the video memory, then proceeds to display the 4 layered GBA backgrounds after applying random tileset displacement. The palette can also be altered. The layers can all be moved together or selected and moved independently to create a parallax scrolling effect.

### Rotation Mode
Same thing but applied to only 2 layers which can however be rotated and scaled. To rotate them independently, select layer with B+Up/Down and enable parallax mode.

### Drawing Mode *(NEW)*
Enables real-time tile and pixel manipulation, turning glitchGBA into an interactive art tool. All layer movement stops and a cursor appears that can be navigated around the screen. You can place tiles from the current tileset, erase content, and manipulate layers in real-time. The cursor shows the currently selected tile with a colored indicator to show its position.

### Console vs Emulator Mode
Define `CONSOLE_MODE` for real GBA hardware, otherwise the ROM is compatible for VBA and most other emulators. This affects movement calculation behavior for platform compatibility.

## Controls

### Mode Switching
- **L + R**: Cycle through modes (Normal → Rotation → Drawing)
- **Start + Select**: Toggle chaos mode (randomizes heavily)
- **A + B + Start + Select**: Reset ROM

---

## Normal Mode Controls

### Basic Actions
- **A**: Pause/Resume
- **B**: Enable/Disable wait for Vertical Sync interrupt
- **Start**: Alter palette
- **Select**: Swap layers movement between uniform and parallax
- **D-Pad (←/→)**: Adjust horizontal speed of current layer
- **D-Pad (↑/↓)**: Adjust vertical speed of current layer
- **R**: Switch to next tileset

### Advanced Actions
- **Start + A**: Toggle remap (layer corruption)
- **Select + A**: Toggle background remap
- **Start + B**: Enable/Disable current layer
- **B + ↑/↓**: Select layer up/down
- **A + B**: Stop all layer movement
- **R + →**: Toggle frame scroll mode
- **Start + L/R**: Adjust tile indices (continuous)

---

## Rotation Mode Controls

### Basic Actions
- **A**: Pause/Resume
- **B**: Enable/Disable wait for Vertical Sync interrupt
- **Start**: Alter palette
- **Select**: Swap layers movement between uniform and parallax
- **D-Pad**: Move transform center
- **R**: Switch to next tileset

### Rotation & Scaling
- **L + A**: Rotate current layer left
- **R + A**: Rotate current layer right
- **L + B**: Zoom current layer out
- **R + B**: Zoom current layer in
- **B + ↑/↓**: Select layer (BG2/BG3 for transforms)

---

## Drawing Mode Controls *(NEW)*

In drawing mode, all layer movement stops and you control a cursor to draw tiles on the screen.

### Basic Actions
- **A**: Place current tile at cursor position
- **B**: Erase tile at cursor position
- **D-Pad (←/→/↑/↓)**: Move cursor
- **L**: Previous tile in tileset
- **R**: Next tile in tileset

### Advanced Drawing Actions
- **Select + B**: Toggle tile rotation
- **Start + B**: Cycle mirror mode (horizontal/vertical flipping)
- **Start + Select**: Clear entire layer
- **B + ↑**: Switch to next layer for drawing

### Drawing Features
- Real-time cursor with tile preview
- Tile placement with rotation and mirroring
- Layer-by-layer editing
- Visual feedback for current tile selection

## Building from source:
### Prerequisites
- Install [devKitPro](https://github.com/devkitPro/installer/releases). Select the GBA development.
- Make sure you have [Python3](https://www.python.org/downloads/) installed.
- Prepare the tilesets in a 256x256 sized **png** file and put it in the graphics folder under whichever name you like.
You can probably exceed this size vertically but be sure to keep the horizontal size 256 pixels. It's not vital but it's probably more organized. Also notice that the size of the tileset should be in multiples of 8 since that's the minimum size of a tile block.
You may theoretically put a lot of tiles but at some point the memory of the GBA will be full and some issues will arise in the compiler. At least a dozen should fit, however

### Build
- Run ```make genereate_headers```. This requires that you run [grit](https://github.com/devkitPro/grit). 
Grit is contained in the devkitpro distribution. Make sure that the devkitPro/tools/bin folder is in your PATH variable.
- Finally, run ```make```.

## Credits
- [Coranac](http://www.coranac.com/) for [grit](https://www.coranac.com/projects/grit/) and the [tonc](http://www.coranac.com/tonc/text/toc.htm) which taught me many things.
- [INFU_AV](https://twitter.com/Infu_av) for his precious feedback on the project.
- [salkinitzor](https://salkinitzor.itch.io/) for the Databrush tool, inspiring the drawing mode in glitchGBA.
- the DevKitPro team for libgba.
