#include <gba.h>
#include <maxmod.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tile1.h"

#define TRUE 1
#define FALSE 0

#define MAP_ADDRESS0 28
#define MAP_ADDRESS1 29
#define MAP_ADDRESS2 30
#define MAP_ADDRESS3 31

int main() {
	
	// IO variable
	int keys_pressed;
	// Frame counter
	int kX[4] = {0, 0, 0, 0};
	int kY[4] = {0, 0, 0, 0};
	// speed X, Y
	int speedX[4] = {0, 0, 0, 0};
	int speedY[4] = {0, 0, 0, 0};
	// your friendly dummy i
	int i;
	
	// Buffer for palette memory address
	short unsigned int * paletteAddress;
	
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
	// are we on a console?
	bool console = FALSE;
	// increaseRate value (depends on hrw/emu)
	int increaseRateX[4] = {0,0,0,0};
	int increaseRateY[4] = {0,0,0,0};
	// randomChange for palette
	char randomChange;
	// backgroundLevel to swap
	int bglevel=0;
	
	// Enable interrupt VBLANK
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	// BACKGROUND
	
	// Load palette
	memcpy(BG_PALETTE, tile1Pal, tile1PalLen);
	// Load Tiles 1
	memcpy(&MAP[0][0], tile1Tiles, tile1TilesLen);
	// Load 1st layer of mapping
	memcpy(&MAP[MAP_ADDRESS0][0], tile1Map, tile1MapLen);
	// Load 2nd Layer of mapping
	memcpy(&MAP[MAP_ADDRESS1][0], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), tile1MapLen);
	// Load 3rd Layer of mapping
	memcpy(&MAP[MAP_ADDRESS2][0], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), tile1MapLen);
	// Load 4th Layer of mapping
	memcpy(&MAP[MAP_ADDRESS3][0], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), tile1MapLen);
	
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
	REG_BG0CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS0) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(0);
	REG_BG1CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS1) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(1);
	REG_BG2CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS2) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(2);
	REG_BG3CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS3) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(3);
	REG_DISPCNT= BG0_ON | BG1_ON | BG2_ON | BG3_ON;
	
	do {
		// Interrupt Wait
		if(interrupt)
			VBlankIntrWait();
		
		// Move screen
		if (!paused){
			// Time ticks
			for(i=0;i<4;i++){
				kX[i]+=speedX[i];
				kY[i]+=speedY[i];
			}
				
			// Emulators have a weird behaviour compared to real hardware,
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
			memcpy(&MAP[MAP_ADDRESS0][(rand()%0xFF)], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), 0x10);
		}
		
		// Remap
		if(!paused & remap){
			// Edit 2nd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS1][(rand()%0xFF)], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), 0x10);
			// Edit 3rd Layer of mapping
			memcpy(&MAP[MAP_ADDRESS2][(rand()%0xFF)], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), 0x10);
			// Edit 4th Layer of mapping
			memcpy(&MAP[MAP_ADDRESS3][(rand()%0xFF)], tile1Tiles+(rand()%(tile1TilesLen-tile1MapLen)), 0x10);
		}
		
		// Mix and change randomly palette (one color at a time)
		if(paletteChange & !paused){
			paletteAddress = BG_PALETTE + (rand() % tile1PalLen);
			randomChange = ((char) *paletteAddress +(rand()%0x0F));
			memcpy(paletteAddress, &randomChange, 1);
			// Must keep background black (background being supposedly first color)
			memset(BG_PALETTE, 0x0000, 2);
		}
		
		// Check keys
		scanKeys();
		keys_pressed = keysDown();
		
		// Pause / resume
		if ( keys_pressed == (KEY_A)) {
			paused = !paused;
		// Turn interrupt on/off
		} else if (keys_pressed == (KEY_B)){
			interrupt=!interrupt;
		// Decrease speed X
		} else if ( keys_pressed == KEY_LEFT) {
			speedX[bglevel]-=1;
		// Increase speed X
		} else if (keys_pressed == KEY_RIGHT) {
			speedX[bglevel]++;
		// Increase speed Y
		} else if (keys_pressed == KEY_UP) {
			speedY[bglevel]++;
		// Decrease speed Y
		} else if (keys_pressed == KEY_DOWN) {
			speedY[bglevel]-=1;
		// Turn palette change on/off
		} else if (keys_pressed == KEY_START){
			paletteChange=!paletteChange;
		// Set speeds to 0
		} else if (keys_pressed == (KEY_A + KEY_B)){
			for (i=0;i<4;i++){
				speedX[i] = 0;
				speedY[i] = 0;
			}
		// Turn parallax on/off
		} else if (keys_pressed == KEY_SELECT){
			parallax = !parallax;
		// Turn remap on/off
		} else if (keys_pressed == (KEY_A + KEY_START)){
			remap = !remap;
		// Turn background remap on/off
		} else if (keys_pressed == (KEY_A + KEY_SELECT)){
			bgremap = !bgremap;
		// Turn console mod on/off
		} else if (keys_pressed == (KEY_B + KEY_SELECT)){
			console = !console;
		// Increase level of background
		} else if (keys_pressed == (KEY_B + KEY_UP)){
			bglevel++;
			if (bglevel==4)
				bglevel = 0;
		// Decrease level of background
		} else if (keys_pressed == (KEY_B + KEY_DOWN)){
			bglevel-=1;
			if (bglevel==-1)
				bglevel = 3;
		// Enable / Disable selected background
		} else if (keys_pressed == (KEY_B + KEY_START)){
			if (bglevel==0){
				REG_DISPCNT^=BG0_ON;
			} else if (bglevel==1){
				REG_DISPCNT^=BG1_ON;
			} else if (bglevel==2){
				REG_DISPCNT^=BG2_ON;
			} else if (bglevel==3){
				REG_DISPCNT^=BG3_ON;
			}
		}
	} while( 1 );
}