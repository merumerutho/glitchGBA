#include "animation.h"
#include "display.h"
#include "graphics.h"
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern bool paused, vsync, parallax, paletteChange, chaos;
extern bool remap, bg_remap, frame_scroll_mode, rotationMode;
extern int x_pos[4], y_pos[4], x_speed[4], y_speed[4];
extern int moveRateX[4], moveRateY[4];
extern int currentTiles, frameCounter;
extern int scale2, scale3, BG2_angular_speed, BG3_angular_speed;
extern BGAffineSource BG_AFF_S2, BG_AFF_S3;
extern BGAffineDest BG_AFF_D2, BG_AFF_D3;

void animation_update_screen_movement(void)
{
	// Move screen
	if (!paused)
	{
		for(int i = 0; i < NUM_LAYERS; i++)
		{
			// Update position
			x_pos[i] += x_speed[i];
			y_pos[i] += y_speed[i];	
			// VBA has a weird behaviour compared to gba and other emus,
			// this is just a quick fix.
#ifdef CONSOLE_MODE
			moveRateX[i] = x_speed[i] ;
			moveRateY[i] = y_speed[i] ;
#else
			moveRateX[i] = x_pos[i] ;
			moveRateY[i] = y_pos[i] ;
#endif
		}
		
		// Parallax => independent movement
		if (parallax)
		{
			if (!rotationMode)
			{
				REG_BG0HOFS += moveRateX[0];
				REG_BG0VOFS += moveRateY[0];
				REG_BG1HOFS += moveRateX[1];
				REG_BG1VOFS += moveRateY[1];
				REG_BG2HOFS += moveRateX[2];
				REG_BG2VOFS += moveRateY[2];
				REG_BG3HOFS += moveRateX[3];
				REG_BG3VOFS += moveRateY[3];
			} 
			else 
			{
				BG_AFF_S2.tX = moveRateX[2];
				BG_AFF_S2.tY = moveRateY[2];
				BG_AFF_S3.tX = moveRateX[3];
				BG_AFF_S3.tY = moveRateY[3];
				BG_AFF_S2.theta += BG2_angular_speed;
				BG_AFF_S3.theta += BG3_angular_speed;
			}
		}	
		// no parallax => every layer moves by the same amount 
		else 
		{
			if (!rotationMode)
			{
				REG_BG0HOFS += moveRateX[0];
				REG_BG0VOFS -= moveRateY[0];
				REG_BG1HOFS += moveRateX[0];
				REG_BG1VOFS -= moveRateY[0];
				REG_BG2HOFS += moveRateX[0];
				REG_BG2VOFS -= moveRateY[0];
				REG_BG3HOFS += moveRateX[0];
				REG_BG3VOFS -= moveRateY[0];
			} 
			else 
			{
				BG_AFF_S2.tX = moveRateX[0];
				BG_AFF_S2.tY = moveRateY[0];
				BG_AFF_S3.tX = moveRateX[0];
				BG_AFF_S3.tY = moveRateY[0];
				BG_AFF_S2.theta += BG2_angular_speed;
				BG_AFF_S3.theta += BG3_angular_speed;
			}
		}
		
		// Set Affine (roto-scale) Data
		if (rotationMode)
		{
			BgAffineSet(&BG_AFF_S2, &BG_AFF_D2, 1);
			BgAffineSet(&BG_AFF_S3, &BG_AFF_D3, 1);
			REG_BG_AFFINE[2] = BG_AFF_D2;
			REG_BG_AFFINE[3] = BG_AFF_D3;
		}
	}
}

void animation_update_remap(void)
{
	// Remap background
	if (!paused && bg_remap)
	{
		memcpy(&MAP[MAP_ADDRESS0][(rand()%0xFF)], tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	}
	
	// Remap non-background layers
	if(!paused && remap){
		// Mess up layers 1,2,3 without getting out of boundaries (MapsLen?)
		memcpy(MAP_LAYER1 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
		memcpy(MAP_LAYER2 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
		memcpy(MAP_LAYER3 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	}
	
	// Chaos mode
	if(chaos)
	{
		// Mess-up layers 1,2,3 allowing getting out of boundaries (MapsLen?)
		memcpy(MAP_LAYER1, tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
		memcpy(MAP_LAYER2, tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
		memcpy(MAP_LAYER3, tilesPointer[0]+(rand()%(TilesLen[0])), MapsLen[0]);
	}
}

void animation_update_frame_scroll(void)
{
	// Frame-scrolling mode - automatically cycles through tilesets
	// VBlank timing handled by main loop for optimal performance
	if(frame_scroll_mode && frameCounter == FRAME_COUNT_SCROLLING_MODE)
	{
		// Reset frame counter
		frameCounter = 0;
		// Increase current tile (wrap to 0)
		currentTiles = (currentTiles < (NTILES-1)) ? (currentTiles+1) : 0;
		// Swap palette unless palette change flag is enabled
		if(!paletteChange)
		{
			dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
		}
		// Swap base tile and background mapping, but not layers 1,2,3
		dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
		dmaCopy(mapsPointer[currentTiles], &MAP[MAP_ADDRESS0][0], MapsLen[currentTiles]);
	}
	else if (frameCounter == FRAME_COUNT_SCROLLING_MODE)
	{
		frameCounter = 0;
	}
}

void animation_update_palette(void)
{
	// Buffer for palette memory address
	unsigned short * paletteAddress;	
	// Mix and change randomly palette (one color at a time)
	if(paletteChange && !paused)
	{
		paletteAddress = BG_PALETTE + (rand() % PalsLen[0]);
		char paletteSwapColor = ((char) *paletteAddress +(rand()%0x0F));
		memcpy(paletteAddress, &paletteSwapColor, 1);
		// Must keep background black (background being supposedly first color)
		memset(BG_PALETTE, 0x0000, 2);
	}
}