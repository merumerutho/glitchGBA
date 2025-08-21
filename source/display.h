#ifndef DISPLAY_H
#define DISPLAY_H

#include <gba.h>

// Background map addresses in VRAM
#define MAP_ADDRESS0 28  // Background layer 0
#define MAP_ADDRESS1 29  // Background layer 1  
#define MAP_ADDRESS2 30  // Background layer 2
#define MAP_ADDRESS3 31  // Background layer 3

#define MAP_LAYER0 &(MAP[MAP_ADDRESS0][0])
#define MAP_LAYER1 &(MAP[MAP_ADDRESS1][0])
#define MAP_LAYER2 &(MAP[MAP_ADDRESS2][0])
#define MAP_LAYER3 &(MAP[MAP_ADDRESS3][0])
#define REG_BG_AFFINE ((BGAffineDest *)(REG_BASE+0x0000))

// GBA screen dimensions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

// Background layer configuration
#define NUM_LAYERS 4
#define MAX_LAYER_INDEX (NUM_LAYERS-1)

// Function declarations
void init_map(void);
void init_tile(void);
void display_init(void);
void display_setup_registers(void);

#endif // DISPLAY_H