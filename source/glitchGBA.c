#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MAP_ADDRESS0 28
#define MAP_ADDRESS1 29
#define MAP_ADDRESS2 30
#define MAP_ADDRESS3 31

#define MOVIEFPS 1

#define DEFAULT_CONSOLE TRUE

#include "graphics.h"

int main() {
	
	// IO variables
	int keys_held, keys_released;
	// Frame counter
	int kX[4] = {0, 1, 2, 3};
	int kY[4] = {0, 1, 2, 3};
	// speed X, Y
	int speedX[4] = {0, 1, 2, 3};
	int speedY[4] = {0, 1, 2, 3};
	// your friendly dummy i
	int i;
	// Buffer for palette memory address
	short unsigned int * paletteAddress;
	// current Palette
	int currentTiles=0;
	// paused?
	bool paused = FALSE;
	// change palette?
	bool paletteChange = FALSE;
	// allow interrupt?
	bool interrupt = TRUE;
	// parallax effect?
	bool parallax = FALSE;
	// remap? (slows down)
	bool remap = FALSE;
	// remap background? (slows down)
	bool bgremap = FALSE;
	// chaos randomizer
	bool chaos = FALSE;
	// movie mode
	bool movie = FALSE;
	// are we on a console?
	bool console = DEFAULT_CONSOLE;
	// Frame counter
	int frameCounter=0;
	// increaseRate value (depends on hrw/emu)
	int increaseRateX[4] = {0,0,0,0};
	int increaseRateY[4] = {0,0,0,0};
	// randomChange for palette
	char randomChange;
	// backgroundLevel to swap
	int bglevel=0;
	// Set current keys pressed to 0 (none)
	keys_held = 0;
	// Multikey control 
	bool multikey = FALSE;
	
	// Enable interrupt VBLANK
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	// BACKGROUND
	// Load palette
	memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
	// Load Tiles 1
	memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
	// Load 1st layer of mapping
	memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
	// Load 2nd Layer of mapping
	memcpy(&MAP[MAP_ADDRESS1][0], tilesPointer[currentTiles]+(rand()%(TilesLen[currentTiles])), MapsLen[currentTiles]);
	// Load 3rd Layer of mapping
	memcpy(&MAP[MAP_ADDRESS2][0], tilesPointer[currentTiles]+(rand()%(TilesLen[currentTiles])), MapsLen[currentTiles]);
	// Load 4th Layer of mapping
	memcpy(&MAP[MAP_ADDRESS3][0], tilesPointer[currentTiles]+(rand()%(TilesLen[currentTiles])), MapsLen[currentTiles]);
	
	// Place BG at random positions
	REG_BG0HOFS= rand() % 240;
	REG_BG0VOFS= rand() % 160;
	REG_BG1HOFS= rand() % 240;
	REG_BG1VOFS= rand() % 160;
	REG_BG2HOFS= rand() % 240;
	REG_BG2VOFS= rand() % 160;
	REG_BG3HOFS= rand() % 240;
	REG_BG3VOFS= rand() % 160;
	
	// Set up registers
	REG_BG0CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS0) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(3);
	REG_BG1CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS1) | BG_256_COLOR | BG_SIZE(1) | BG_PRIORITY(2);
	REG_BG2CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS2) | BG_256_COLOR | BG_SIZE(2) | BG_PRIORITY(1);
	REG_BG3CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS3) | BG_256_COLOR | BG_SIZE(3) | BG_PRIORITY(0);
	REG_DISPCNT= BG0_ON | BG1_ON | BG2_ON | BG3_ON;
	
	do {
		// Interrupt Wait
		if(interrupt)
			VBlankIntrWait();
		
		frameCounter++;
		
		// Move screen
		if (!paused){
			// Time ticks
			for(i=0;i<4;i++){
				kX[i]+=speedX[i];
				kY[i]+=speedY[i];
			}
				
			// VBA has a weird behaviour compared to gba and other emus,
			// this is just a quick fix.
			if (console){
				for(i=0;i<4;i++){
					increaseRateX[i] = speedX[i];
					increaseRateY[i] = speedY[i];
				}
			}else{
				for(i=0;i<4;i++){
					increaseRateX[i] = kX[i];
					increaseRateY[i] = kY[i];
				}
			}
			
			// If parallax enabled
			if(parallax){
				REG_BG0HOFS+=increaseRateX[0];
				REG_BG0VOFS+=increaseRateY[0];
				REG_BG1HOFS+=increaseRateX[1];
				REG_BG1VOFS+=increaseRateY[1];
				REG_BG2HOFS+=increaseRateX[2];
				REG_BG2VOFS+=increaseRateY[2];
				REG_BG3HOFS+=increaseRateX[3];
				REG_BG3VOFS+=increaseRateY[3];
			// else everybody moves the same
			} else {
				REG_BG0HOFS+=increaseRateX[0];
				REG_BG0VOFS-=increaseRateY[0];
				REG_BG1HOFS+=increaseRateX[0];
				REG_BG1VOFS-=increaseRateY[0];
				REG_BG2HOFS+=increaseRateX[0];
				REG_BG2VOFS-=increaseRateY[0];
				REG_BG3HOFS+=increaseRateX[0];
				REG_BG3VOFS-=increaseRateY[0];
			}
			
		}
		
		// Remap background
		if (bgremap){
			memcpy(&MAP[MAP_ADDRESS0][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
		}
		
		// Remap
		if(!paused & remap){
			// Edit 2nd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS1][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
			// Edit 3rd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS2][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
			// Edit 4th Layer of mapping
			memcpy(&MAP[MAP_ADDRESS3][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
		}
		
		// Chaos mode
		if(chaos){
			// Load 2nd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS1][0], tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
			// Load 3rd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS2][0], tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
			// Load 4th Layer of mapping
			memcpy(&MAP[MAP_ADDRESS3][0], tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
		}
		
		// Movie mode
		if(movie && frameCounter== MOVIEFPS){
			frameCounter = 0;
			currentTiles++;
			if(currentTiles==NTILES)
				currentTiles = 0;
			if(!paletteChange)
				memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
			memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
			memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
		} else if (frameCounter == MOVIEFPS){
			frameCounter = 0;
		}
		
		// Mix and change randomly palette (one color at a time)
		if(paletteChange & !paused){
			paletteAddress = BG_PALETTE + (rand() % PalsLen[0]);
			randomChange = ((char) *paletteAddress +(rand()%0x0F));
			memcpy(paletteAddress, &randomChange, 1);
			// Must keep background black (background being supposedly first color)
			memset(BG_PALETTE, 0x0000, 2);
		}
		
		// Check current key status
		scanKeys();
		// Get Pressed Keys
		keys_held = keysHeld();
		// Get Released Keys
		keys_released = keysUp();
		
		// Re-set value of multikey when no keys pressed
		if ((keys_held == 0) && (keys_released == 0))
			multikey = FALSE;
		
		/* 
			CONTROLS SECTION
			Singlekeys
		*/
		
		if (keys_held == 0 && multikey == FALSE) {
			
			// Pause / resume
			if (keys_released == (KEY_A)) {
				paused=!paused;

			// Turn interrupt on/off
			} else if (keys_released == (KEY_B)){
				interrupt=!interrupt;
				
			// Decrease speed X
			} else if ( keys_released == KEY_LEFT) {
				speedX[bglevel]-=1;
				
			// Increase speed X
			} else if (keys_released == KEY_RIGHT) {
				speedX[bglevel]++;
				
			// Increase speed Y
			} else if (keys_released == KEY_UP) {
				speedY[bglevel]++;
				
			// Decrease speed Y
			} else if (keys_released == KEY_DOWN) {
				speedY[bglevel]-=1;
				
			// Turn palette change on/off
			} else if (keys_released == KEY_START){
				paletteChange=!paletteChange;
				
			// Turn parallax on/off
			} else if (keys_released == KEY_SELECT){
				parallax = !parallax;
			
			// Switch to right tiles only 
			} else if (keys_released == (KEY_R)){
				currentTiles++;
				if (currentTiles==NTILES)
					currentTiles = 0;
				memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
				memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
				memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
				memcpy(&MAP[MAP_ADDRESS1][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
				memcpy(&MAP[MAP_ADDRESS2][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
				memcpy(&MAP[MAP_ADDRESS3][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);

			// Switch to left tiles only
			} else if (keys_released == (KEY_L)){
				currentTiles--;
				if (currentTiles==-1)
					currentTiles = NTILES-1;
				memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
				memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
				memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
				memcpy(&MAP[MAP_ADDRESS1][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
				memcpy(&MAP[MAP_ADDRESS2][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
				memcpy(&MAP[MAP_ADDRESS3][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);	
			}
		}
		
		/*
			CONTROLS SECTION
			Multikeys
		*/
		
		// Set speeds to 0
		if ((keys_held & KEY_A) && (keys_released & KEY_B)){
			multikey = TRUE;
			for (i=0;i<4;i++){
				speedX[i] = 0;
				speedY[i] = 0;
			}
			
		// Enable / Disable selected background
		} else if ((keys_held & KEY_START) && (keys_released & KEY_B)){
			multikey = TRUE;
			if (bglevel==0){
				REG_DISPCNT^=BG0_ON;
			} else if (bglevel==1){
				REG_DISPCNT^=BG1_ON;
			} else if (bglevel==2){
				REG_DISPCNT^=BG2_ON;
			} else if (bglevel==3){
				REG_DISPCNT^=BG3_ON;
			}
			
		// Turn remap on/off
		} else if ((keys_held & KEY_START) && (keys_released & KEY_A)){
			multikey = TRUE;
			remap = !remap;
		
		// Turn background remap on/off
		} else if ((keys_held & KEY_SELECT) && (keys_released & KEY_A)){
			multikey = TRUE;
			bgremap = !bgremap;
			
		// Turn console mod on/off
		} else if ((keys_held & KEY_SELECT) && (keys_released & KEY_B)){
			multikey = TRUE;
			console = !console;
			
		// Increase level of background
		} else if ((keys_held & KEY_B) && (keys_released & KEY_UP)){
			multikey = TRUE;
			bglevel++;
			if (bglevel==4)
				bglevel = 0;
			
		// Decrease level of background
		} else if ((keys_held & KEY_B) && (keys_released & KEY_DOWN)){
			multikey = TRUE;
			bglevel-=1;
			if (bglevel==-1)
				bglevel = 3;
			
		// Enable / Disable chaos mode
		} else if (keys_held == (KEY_START + KEY_SELECT)){
			multikey = TRUE;
			chaos=!chaos;
			if(chaos){
				interrupt=FALSE;
			} else {
				interrupt=TRUE;
			}
			
		// Switch to right tiles & palette
		} else if ((keys_held & KEY_R) && (keys_released & KEY_A)){
			multikey = TRUE;
			currentTiles++;
			if (currentTiles==NTILES)
				currentTiles = 0;
			memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
			
		// Switch to left tiles & palette
		} else if ((keys_held & KEY_L) && (keys_released & KEY_A)){
			multikey = TRUE;
			currentTiles--;
			if (currentTiles==-1)
				currentTiles = NTILES-1;
			memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
			
		// Enable / Disable movie mode
		} else if ((keys_held & KEY_R) && (keys_released & KEY_RIGHT)){
			multikey = TRUE;
			movie=!movie;
			
		// RESET TO DEFAULT
		} else if (keys_held == (KEY_A + KEY_B + KEY_START + KEY_SELECT)){
			multikey = TRUE;
			currentTiles = 0;
			memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
			memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
			memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
			memcpy(&MAP[MAP_ADDRESS1][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
			memcpy(&MAP[MAP_ADDRESS2][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
			memcpy(&MAP[MAP_ADDRESS3][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
			for (i=0;i<4;i++){
				speedX[i] = 0;
				speedY[i] = 0;
			}
			chaos = FALSE;
			bgremap = FALSE;
			remap = FALSE;
			parallax = FALSE;
			console = FALSE;
			movie = FALSE;
			frameCounter=0;
			paused = FALSE;
			interrupt = TRUE;
		}
		
	} while( 1 );
}