#include "input.h"
#include "display.h"
#include "graphics.h"
#include "drawing.h"
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern bool paused, vsync, parallax, paletteChange, chaos;
extern bool remap, bg_remap, frame_scroll_mode, rotationMode;
extern DrawingSystem drawing;
extern int x_speed[4], y_speed[4];
extern int bglevel, currentTiles;
extern int scale2, scale3, BG2_angular_speed, BG3_angular_speed;
extern BGAffineSource BG_AFF_S2, BG_AFF_S3;

// Forward declaration for reset function
extern void init(void);
extern void cycle_app_mode(void);
extern void drawing_cycle_mirror_mode(void);

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
	// Handle continuous input for drawing mode (always check held keys)
	if (drawing.enabled) {
		drawing_handle_continuous_input(input->keys_held);
	}
	
	if (!input->keys_held && input->multikey == FALSE) 
	{	
		// Drawing mode has different controls
		if (drawing.enabled) {
			switch(input->keys_released)
			{
				// Place tile
				case KEY_A:
					drawing_place_tile();
					break;
				// Erase tile
				case KEY_B:
					drawing_erase_tile();
					break;
				// Move cursor
				case KEY_LEFT:
					drawing_move_cursor(-1, 0);
					break;
				case KEY_RIGHT:
					drawing_move_cursor(1, 0);
					break;
				case KEY_UP:
					drawing_move_cursor(0, -1);
					break;
				case KEY_DOWN:
					drawing_move_cursor(0, 1);
					break;
				// Next tile
				case KEY_R:
					drawing_next_tile();
					break;
				// Previous tile
				case KEY_L:
					drawing_prev_tile();
					break;
				// Next layer
				case KEY_SELECT:
					drawing_next_layer();
					break;
				// Clear current layer
				case KEY_START:
					drawing_clear_layer();
					break;
				default:
					break;
			}
		} else {
			// Normal mode controls
			switch(input->keys_released){
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
				drawing.tile.selected_tile = 0;
				dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
				//dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
				//init_map();
				break;
			// Swap to left tileset map
			case KEY_L:
				currentTiles = (currentTiles > 0) ? (currentTiles-1) : (NTILES - 1);
				drawing.tile.selected_tile = 0;
				dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
				dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
				//init_map();
				break;
			default:
				break;
			}
		}
	}
}

void input_handle_multi_keys(InputState* input)
{
	// ===== GLOBAL MODE SWITCHING =====
	// Unified mode switching: L+R cycles through Normal->Rotation->Drawing
	if ((input->keys_held & KEY_L) && (input->keys_released & KEY_R))
	{
		input->multikey = TRUE;
		cycle_app_mode();
		paused = drawing.enabled; // Stop movement in drawing mode, resume otherwise
	}
	
	// ===== GLOBAL SYSTEM COMMANDS =====
	// Reset all settings
	else if (input->keys_held == (KEY_A + KEY_B + KEY_START + KEY_SELECT))
	{
		input->multikey = TRUE;
		init();	
	}
	// Enable / Disable chaos mode
	else if (input->keys_held == (KEY_START + KEY_SELECT))
	{
		input->multikey = TRUE;
		vsync = !(chaos = !chaos);
	}
	
	// ===== DRAWING MODE COMMANDS =====
	// Toggle tile rotation in drawing mode (SELECT+B)
	else if (drawing.enabled && (input->keys_held & KEY_SELECT) && (input->keys_released & KEY_B))
	{
		input->multikey = TRUE;
		drawing_toggle_rotation();
	}
	// Cycle through mirror modes in drawing mode (START+B)
	else if (drawing.enabled && (input->keys_held & KEY_START) && (input->keys_released & KEY_B))
	{
		input->multikey = TRUE;
		drawing_cycle_mirror_mode();
	}
	
	// ===== ROTATION MODE COMMANDS =====
	// Rotate right (R+A)
	else if (rotationMode && (input->keys_held == (KEY_R + KEY_A)))
	{
		input->multikey = TRUE;
		((bglevel % 2) == 0) ? (BG2_angular_speed -= 1) : (BG3_angular_speed -= 1);
	}
	// Rotate left (L+A)
	else if (rotationMode && (input->keys_held == (KEY_L + KEY_A)))
	{
		input->multikey = TRUE;
		((bglevel % 2) == 0) ? (BG2_angular_speed += 1) : (BG3_angular_speed += 1);
	}
	// Zoom in (R+B)
	else if (rotationMode && (input->keys_held == (KEY_R + KEY_B)))
	{
		input->multikey = TRUE;
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
	// Zoom out (L+B)
	else if (rotationMode && (input->keys_held == (KEY_L + KEY_B)))
	{
		input->multikey = TRUE;
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
	
	// ===== NORMAL MODE COMMANDS =====
	// Set layer speeds to 0 without resetting (A+B)
	else if (!drawing.enabled && (input->keys_held & KEY_A) && (input->keys_released & KEY_B))
	{
		input->multikey = TRUE;
		for (int i=0; i<NUM_LAYERS; i++)
		{
			x_speed[i] = 0;
			y_speed[i] = 0;
		}
	}
	// Enable / Disable selected background layer (START+B)
	else if (!drawing.enabled && (input->keys_held & KEY_START) && (input->keys_released & KEY_B))
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
	// Turn tileset remap on/off (START+A)
	else if (!drawing.enabled && (input->keys_held & KEY_START) && (input->keys_released & KEY_A))
	{
		input->multikey = TRUE;
		remap = !remap;
	}
	// Turn background remap on/off (SELECT+A)
	else if (!drawing.enabled && (input->keys_held & KEY_SELECT) && (input->keys_released & KEY_A))
	{
		input->multikey = TRUE;
		bg_remap = !bg_remap;
	}	
	// Increase background layer level (B+UP)
	else if (!drawing.enabled && (input->keys_held & KEY_B) && (input->keys_released & KEY_UP))
	{
		input->multikey = TRUE;
		bglevel = (bglevel < (NUM_LAYERS-1)) ? (bglevel + 1) : 0;
	}	
	// Decrease background layer level (B+DOWN)
	else if (!drawing.enabled && (input->keys_held & KEY_B) && (input->keys_released & KEY_DOWN))
	{
		input->multikey = TRUE;
		bglevel = (bglevel > 0) ? bglevel-1 : (NUM_LAYERS-1);
	}
	// Enable / Disable frame scroll mode (R+RIGHT)
	else if (!drawing.enabled && (input->keys_held & KEY_R) && (input->keys_released & KEY_RIGHT))
	{
		input->multikey = TRUE;
		frame_scroll_mode = !frame_scroll_mode;
	}	
}