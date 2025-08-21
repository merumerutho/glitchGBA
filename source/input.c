#include "input.h"
#include "display.h"
#include "graphics.h"
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern bool paused, vsync, parallax, paletteChange, chaos;
extern bool remap, bg_remap, frame_scroll_mode, rotationMode;
extern int x_speed[4], y_speed[4];
extern int bglevel, currentTiles;
extern int scale2, scale3, BG2_angular_speed, BG3_angular_speed;
extern BGAffineSource BG_AFF_S2, BG_AFF_S3;

// Forward declaration for reset function
extern void init(void);

void input_init(void)
{
	// Nothing special needed for input initialization
}

void input_update(InputState* input)
{
	// Check current key status
	scanKeys();
	// Get Pressed Keys
	input->keys_held = keysHeld();
	// Get Released Keys
	input->keys_released = keysUp();
	
	// Re-set value of multikey when no keys are pressed
	if ((!input->keys_held) && (!input->keys_released))
	{
		input->multikey = FALSE;
	}
}

void input_handle_single_keys(InputState* input)
{
	if (!input->keys_held && input->multikey == FALSE) 
	{	
		switch(input->keys_released)
		{
			// Pause / unpause
			case KEY_A:
				paused = !paused;
				break;
			// Set Vsync 
			case KEY_B:
				vsync = !vsync;
				break;
			// Set horizontal speed
			case KEY_LEFT:
				x_speed[bglevel]--;
				break;
			case KEY_RIGHT:
				x_speed[bglevel]++;
				break;
			// Set vertical speed
			case KEY_UP:
				y_speed[bglevel]++;
				break;
			case KEY_DOWN:
				y_speed[bglevel]--;
				break;
			// Enable/disable palette change
			case KEY_START:
				paletteChange = !paletteChange;
				break;
			// Enable/disable parallax
			case KEY_SELECT:
				parallax = !parallax;
				break;
			// Swap to right tileset map 
			case KEY_R:
				currentTiles = (currentTiles < (NTILES - 1)) ? (currentTiles+1) : 0;
				dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
				dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
				init_map();
				break;
			// Swap to left tileset map
			case KEY_L:
				currentTiles = (currentTiles > 0) ? (currentTiles-1) : (NTILES - 1);
				dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
				dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
				init_map();
				break;
			default:
				break;
		}
	}
}

void input_handle_multi_keys(InputState* input)
{
	// Reset status
	if (input->keys_held == (KEY_A + KEY_B + KEY_START + KEY_SELECT))
	{
		input->multikey = TRUE;
		init();	
	}
	// Set speeds to 0 without resetting
	else if ((input->keys_held & KEY_A) && (input->keys_released & KEY_B))
	{
		input->multikey = TRUE;
		for (int i=0; i<NUM_LAYERS; i++)
		{
			x_speed[i] = 0;
			y_speed[i] = 0;
		}
	}
	// Enable / Disable selected background
	else if ((input->keys_held & KEY_START) && (input->keys_released & KEY_B))
	{
		input->multikey = TRUE;
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
	// Turn remap on/off
	else if ((input->keys_held & KEY_START) && (input->keys_released & KEY_A))
	{
		input->multikey = TRUE;
		remap = !remap;
	}
	// Turn background remap on/off		
	else if ((input->keys_held & KEY_SELECT) && (input->keys_released & KEY_A))
	{
		input->multikey = TRUE;
		bg_remap = !bg_remap;
	}	
	// Increase level of background
	else if ((input->keys_held & KEY_B) && (input->keys_released & KEY_UP))
	{
		input->multikey = TRUE;
		bglevel = (bglevel < (NUM_LAYERS-1)) ? (bglevel + 1) : 0;
	}	
	// Decrease level of background
	else if ((input->keys_held & KEY_B) && (input->keys_released & KEY_DOWN))
	{
		input->multikey = TRUE;
		bglevel = (bglevel > 0) ? bglevel-1 : (NUM_LAYERS-1);
	}	
	// Enable / Disable chaos mode
	else if (input->keys_held == (KEY_START + KEY_SELECT))
	{
		input->multikey = TRUE;
		chaos = !chaos;
		vsync = !chaos;
	}
	// In rotation mode, rotate right
	else if ((input->keys_held == (KEY_R + KEY_A)))
	{
		input->multikey = TRUE;
		if (rotationMode)
		{
			if ((bglevel % 2) == 0)
			{
				BG2_angular_speed -= 1;
			} 
			else 
			{
				BG3_angular_speed -= 1;
			}
		}
	}
	// In rotation mode, rotate left
	else if ((input->keys_held == (KEY_L + KEY_A)))
	{
		input->multikey = TRUE;
		if (rotationMode)
		{
			if ((bglevel % 2) == 0)
			{
				BG2_angular_speed += 1;
			} 
			else 
			{
				BG3_angular_speed += 1;
			}
		}
	}
	// Zoom in (rotation mode)
	else if ((input->keys_held == (KEY_R + KEY_B)))
	{
		input->multikey = TRUE;
		if (rotationMode)
		{
			if ((bglevel % 2) == 0)
			{
				scale2 += 1;
				BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
			} 
			else 
			{
				scale3 += 1;
				BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
			}
		}
	} 
	// Zoom out (rotation mode)
	else if ((input->keys_held == (KEY_L + KEY_B)))
	{
		input->multikey = TRUE;
		if (rotationMode)
		{
			if ((bglevel % 2) == 0)
			{
				scale2 -= 1;
				BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
			} 
			else 
			{
				scale3 -= 1;
				BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
			}
		}
	}
	// Enable / Disable frame_scroll_mode
	else if ((input->keys_held & KEY_R) && (input->keys_released & KEY_RIGHT))
	{
		input->multikey = TRUE;
		frame_scroll_mode = !frame_scroll_mode;
	}	
	// Switch rotation mode - normal mode
	else if ((input->keys_held & KEY_L) && (input->keys_released & KEY_R)) 
	{
		input->multikey = TRUE;
		rotationMode=!rotationMode;
		if (rotationMode)
		{
			REG_DISPCNT = MODE_2 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
			dmaCopy(mapsPointer[currentTiles], &MAP[MAP_ADDRESS2][0], MapsLen[currentTiles]);
			dmaCopy(tilesPointer[currentTiles]+(rand()%(TilesLen[currentTiles])), &MAP[MAP_ADDRESS3][0], MapsLen[currentTiles]);
		} 
		else 
		{
			REG_DISPCNT = BG0_ON | BG1_ON | BG2_ON | BG3_ON;
			init_map();
			bglevel = 0;
		}
	}
}