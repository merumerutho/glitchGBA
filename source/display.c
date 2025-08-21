#include "display.h"
#include "graphics.h"
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern int currentTiles;
extern bool rotationMode;

void init_map(void)
{
	// Reset glitch on tileset
	memcpy(MAP_LAYER0, mapsPointer[currentTiles], MapsLen[currentTiles]);
	memcpy(MAP_LAYER1 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	memcpy(MAP_LAYER2 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	memcpy(MAP_LAYER3 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
}

void init_tile(void)
{
	dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
	dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
	init_map();
}

void display_init(void)
{
	// Initialize background tiles and maps
	init_tile();
	
	// Place map at random positions
	REG_BG0HOFS = rand() % SCREEN_WIDTH;
	REG_BG0VOFS = rand() % SCREEN_HEIGHT;
	REG_BG1HOFS = rand() % SCREEN_WIDTH;
	REG_BG1VOFS = rand() % SCREEN_HEIGHT;
	REG_BG2HOFS = rand() % SCREEN_WIDTH;
	REG_BG2VOFS = rand() % SCREEN_HEIGHT;
	REG_BG3HOFS = rand() % SCREEN_WIDTH;
	REG_BG3VOFS = rand() % SCREEN_HEIGHT;
}

void display_setup_registers(void)
{
	// Set up background control registers
	REG_BG0CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS0) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(3);
	REG_BG1CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS1) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(2);
	REG_BG2CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS2) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(1) | BG_WRAP;
	REG_BG3CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS3) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(0) | BG_WRAP;
	
	REG_DISPCNT = (rotationMode << 1) | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
}