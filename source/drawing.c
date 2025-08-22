#include "drawing.h"
#include "display.h"
#include "graphics.h"
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern int draw_cursor_x, draw_cursor_y;
extern int draw_selected_tile, draw_active_layer;
extern int draw_cursor_blink;
extern int draw_prev_cursor_x, draw_prev_cursor_y;
extern u16 draw_original_tile;
extern u16 draw_mirror_original_tiles[3];
extern TileRotation draw_tile_rotation;
extern MirrorMode draw_mirror_mode;
extern int draw_key_repeat_timer, draw_last_held_keys;
extern bool drawingMode;
extern int currentTiles;

// Forward declarations
extern void init_map(void);

void drawing_init(void)
{
	// Initialize drawing cursor to center of screen
	draw_cursor_x = DRAW_CURSOR_MAX_X / 2;
	draw_cursor_y = DRAW_CURSOR_MAX_Y / 2;
	//draw_selected_tile = 0;
	draw_active_layer = 0;
	draw_cursor_blink = 0;
	draw_prev_cursor_x = -1;
	draw_prev_cursor_y = -1;
	draw_original_tile = 0;
	draw_mirror_original_tiles[0] = draw_mirror_original_tiles[1] = draw_mirror_original_tiles[2] = 0;
	draw_tile_rotation = TILE_ROT_0;
	draw_mirror_mode = MIRROR_NONE;
	draw_key_repeat_timer = 0;
	draw_last_held_keys = 0;
}

void drawing_update(void)
{
	if (!drawingMode) return;
	
	// Update cursor blink animation
	draw_cursor_blink++;
	if (draw_cursor_blink >= DRAW_CURSOR_BLINK_SPEED * 2) {
		draw_cursor_blink = 0;
	}
	
	// Render cursor
	drawing_render_cursor();
}

void drawing_move_cursor(int dx, int dy)
{
	if (!drawingMode) return;
	
	// Update cursor position with bounds checking
	draw_cursor_x += dx;
	draw_cursor_y += dy;
	
	// Wrap around screen edges
	draw_cursor_x = (draw_cursor_x < 0) ? DRAW_CURSOR_MAX_X : (draw_cursor_x > DRAW_CURSOR_MAX_X) ? 0 : draw_cursor_x;
	draw_cursor_y = (draw_cursor_y < 0) ? DRAW_CURSOR_MAX_Y : (draw_cursor_y > DRAW_CURSOR_MAX_Y) ? 0 : draw_cursor_y;
}

void drawing_place_tile(void)
{
	if (!drawingMode) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (draw_active_layer) {
		case 0:
			layer_ptr = (u16*)MAP_LAYER0;
			break;
		case 1:
			layer_ptr = (u16*)MAP_LAYER1;
			break;
		case 2:
			layer_ptr = (u16*)MAP_LAYER2;
			break;
		case 3:
			layer_ptr = (u16*)MAP_LAYER3;
			break;
		default:
			return;
	}
	
	// Apply rotation using hardware flip bits
	u16 tile_entry = draw_selected_tile;
	switch (draw_tile_rotation) {
		case TILE_ROT_0:
			// No modification needed
			break;
		case TILE_ROT_180:
			// Apply horizontal and vertical flip bits (bits 10 and 11)
			tile_entry |= (1 << 10) | (1 << 11);
			break;
	}
	
	// Place tile at primary cursor position
	int map_pos = draw_cursor_y * 32 + draw_cursor_x;
	memcpy(layer_ptr + map_pos, &tile_entry, sizeof(u16));
	
	// Place tiles at mirrored positions based on mirror mode
	if (draw_mirror_mode != MIRROR_NONE) {
		// Place tile at horizontal mirror
		if (draw_mirror_mode == MIRROR_HORIZONTAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
		
		// Place tile at vertical mirror
		if (draw_mirror_mode == MIRROR_VERTICAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
		
		// Place tile at both mirrors (diagonal mirror)
		if (draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
	}
}

void drawing_erase_tile(void)
{
	if (!drawingMode) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (draw_active_layer) {
		case 0:
			layer_ptr = (u16*)MAP_LAYER0;
			break;
		case 1:
			layer_ptr = (u16*)MAP_LAYER1;
			break;
		case 2:
			layer_ptr = (u16*)MAP_LAYER2;
			break;
		case 3:
			layer_ptr = (u16*)MAP_LAYER3;
			break;
		default:
			return;
	}
	
	u16 zero_tile = 0;
	
	// Erase tile at primary cursor position
	int map_pos = draw_cursor_y * 32 + draw_cursor_x;
	memcpy(layer_ptr + map_pos, &zero_tile, sizeof(u16));
	
	// Erase tiles at mirrored positions based on mirror mode
	if (draw_mirror_mode != MIRROR_NONE) {
		// Erase tile at horizontal mirror
		if (draw_mirror_mode == MIRROR_HORIZONTAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
		
		// Erase tile at vertical mirror
		if (draw_mirror_mode == MIRROR_VERTICAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
		
		// Erase tile at both mirrors (diagonal mirror)
		if (draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
	}
}

void drawing_clear_layer(void)
{
	if (!drawingMode) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (draw_active_layer) {
		case 0:
			layer_ptr = (u16*)MAP_LAYER0;
			break;
		case 1:
			layer_ptr = (u16*)MAP_LAYER1;
			break;
		case 2:
			layer_ptr = (u16*)MAP_LAYER2;
			break;
		case 3:
			layer_ptr = (u16*)MAP_LAYER3;
			break;
		default:
			return;
	}
	
	// Clear entire layer (32x32 tiles = 1024 tiles)
	memset(layer_ptr, 0, 32 * 32 * sizeof(u16));
	
	// Reset the original tile since we cleared everything
	draw_original_tile = 0;
}

void drawing_next_tile(void)
{
	if (!drawingMode) return;
	
	// Get the number of tiles in current tileset
	int max_tiles = TilesLen[currentTiles] / 64; // Each tile is 64 bytes (8x8x8bit)
	
	draw_selected_tile = (++draw_selected_tile >= max_tiles) ? 0 : draw_selected_tile;
}

void drawing_prev_tile(void)
{
	if (!drawingMode) return;
	
	// Get the number of tiles in current tileset
	int max_tiles = TilesLen[currentTiles] / 64; // Each tile is 64 bytes (8x8x8bit)
	
	draw_selected_tile = (--draw_selected_tile < 0) ? (max_tiles - 1) : draw_selected_tile;
}

void drawing_next_layer(void)
{
	if (!drawingMode) return;
	
	draw_active_layer = (++draw_active_layer > 3) ? 0 : draw_active_layer;
}

void drawing_toggle_rotation(void)
{
	if (!drawingMode) return;
	
	// Toggle between 0° and 180° rotation
	draw_tile_rotation = (draw_tile_rotation == TILE_ROT_0) ? TILE_ROT_180 : TILE_ROT_0;
}

void drawing_cycle_mirror_mode(void)
{
	if (!drawingMode) return;
	
	// Cycle through mirror modes: None -> Horizontal -> Vertical -> Both -> None
	switch (draw_mirror_mode) {
		case MIRROR_NONE:
			draw_mirror_mode = MIRROR_HORIZONTAL;
			break;
		case MIRROR_HORIZONTAL:
			draw_mirror_mode = MIRROR_VERTICAL;
			break;
		case MIRROR_VERTICAL:
			draw_mirror_mode = MIRROR_BOTH;
			break;
		case MIRROR_BOTH:
			draw_mirror_mode = MIRROR_NONE;
			break;
	}
}

void drawing_get_mirror_positions(int x, int y, int *mirror_x, int *mirror_y, MirrorMode mode)
{
	// Calculate center of the mapping space (30x20 tiles)
	int center_x = DRAW_CURSOR_MAX_X / 2;  // 15
	int center_y = DRAW_CURSOR_MAX_Y / 2;  // 9 (for 20 tile height)
	
	switch (mode) {
		case MIRROR_NONE:
			*mirror_x = x;
			*mirror_y = y;
			break;
		case MIRROR_HORIZONTAL:
			// Mirror horizontally around center
			*mirror_x = center_x + (center_x - x);
			*mirror_y = y;
			break;
		case MIRROR_VERTICAL:
			// Mirror vertically around center  
			*mirror_x = x;
			*mirror_y = center_y + (center_y - y);
			break;
		case MIRROR_BOTH:
			// Mirror both horizontally and vertically
			*mirror_x = center_x + (center_x - x);
			*mirror_y = center_y + (center_y - y);
			break;
	}
	
	// Ensure mirrored coordinates stay within bounds
	if (*mirror_x < 0) *mirror_x = 0;
	if (*mirror_x > DRAW_CURSOR_MAX_X) *mirror_x = DRAW_CURSOR_MAX_X;
	if (*mirror_y < 0) *mirror_y = 0;
	if (*mirror_y > DRAW_CURSOR_MAX_Y) *mirror_y = DRAW_CURSOR_MAX_Y;
}

void drawing_handle_continuous_input(int keys_held)
{
	if (!drawingMode) return;
	
	// Check if the held keys changed
	if (keys_held != draw_last_held_keys) {
		draw_key_repeat_timer = 0;
		draw_last_held_keys = keys_held;
		return;
	}
	
	// If no keys held, reset timer
	if (keys_held == 0) {
		draw_key_repeat_timer = 0;
		return;
	}
	
	// Increment repeat timer
	draw_key_repeat_timer++;
	
	// Check if we should trigger repeat actions
	bool should_repeat = FALSE;
	
	if (draw_key_repeat_timer == KEY_REPEAT_DELAY) {
		// Initial repeat after delay
		should_repeat = TRUE;
	} else if (draw_key_repeat_timer > KEY_REPEAT_DELAY && 
			   (draw_key_repeat_timer - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
		// Subsequent repeats at regular intervals
		should_repeat = TRUE;
	}
	
	if (!should_repeat) return;
	
	// Handle continuous tile selection (L/R)
	if ((keys_held & KEY_L) && !(keys_held & KEY_R)) {
		drawing_prev_tile();
	} else if ((keys_held & KEY_R) && !(keys_held & KEY_L)) {
		drawing_next_tile();
	}
	
	// Handle continuous placement/movement (A + directions)
	if (keys_held & KEY_A) {
		// Place tile at current position
		drawing_place_tile();
		
		// Then move cursor and place at new position
		if (keys_held & KEY_LEFT) {
			drawing_move_cursor(-1, 0);
			drawing_place_tile();
		} else if (keys_held & KEY_RIGHT) {
			drawing_move_cursor(1, 0);
			drawing_place_tile();
		} else if (keys_held & KEY_UP) {
			drawing_move_cursor(0, -1);
			drawing_place_tile();
		} else if (keys_held & KEY_DOWN) {
			drawing_move_cursor(0, 1);
			drawing_place_tile();
		}
	}
	// Handle continuous erase/movement (B + directions)
	else if (keys_held & KEY_B) {
		// Erase tile at current position
		drawing_erase_tile();
		
		// Then move cursor and erase at new position
		if (keys_held & KEY_LEFT) {
			drawing_move_cursor(-1, 0);
			drawing_erase_tile();
		} else if (keys_held & KEY_RIGHT) {
			drawing_move_cursor(1, 0);
			drawing_erase_tile();
		} else if (keys_held & KEY_UP) {
			drawing_move_cursor(0, -1);
			drawing_erase_tile();
		} else if (keys_held & KEY_DOWN) {
			drawing_move_cursor(0, 1);
			drawing_erase_tile();
		}
	}
	// Handle continuous movement only (directions without A/B)
	else {
		if (keys_held & KEY_LEFT) {
			drawing_move_cursor(-1, 0);
		} else if (keys_held & KEY_RIGHT) {
			drawing_move_cursor(1, 0);
		} else if (keys_held & KEY_UP) {
			drawing_move_cursor(0, -1);
		} else if (keys_held & KEY_DOWN) {
			drawing_move_cursor(0, 1);
		}
	}
}

void drawing_render_cursor(void)
{
	if (!drawingMode) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (draw_active_layer) {
		case 0:
			layer_ptr = (u16*)MAP_LAYER0;
			break;
		case 1:
			layer_ptr = (u16*)MAP_LAYER1;
			break;
		case 2:
			layer_ptr = (u16*)MAP_LAYER2;
			break;
		case 3:
			layer_ptr = (u16*)MAP_LAYER3;
			break;
		default:
			return;
	}
	
	// If cursor moved, restore original tiles at all previous positions
	if (draw_prev_cursor_x != -1 && draw_prev_cursor_y != -1 && 
		(draw_prev_cursor_x != draw_cursor_x || draw_prev_cursor_y != draw_cursor_y)) {
		
		// Restore primary position
		int prev_map_pos = draw_prev_cursor_y * 32 + draw_prev_cursor_x;
		memcpy(layer_ptr + prev_map_pos, &draw_original_tile, sizeof(u16));
		
		// Restore mirrored positions using their saved original tiles
		if (draw_mirror_mode != MIRROR_NONE) {
			// Restore horizontal mirror
			if (draw_mirror_mode == MIRROR_HORIZONTAL || draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_prev_cursor_x, draw_prev_cursor_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
				if (mirror_x != draw_prev_cursor_x || mirror_y != draw_prev_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &draw_mirror_original_tiles[0], sizeof(u16));
				}
			}
			
			// Restore vertical mirror  
			if (draw_mirror_mode == MIRROR_VERTICAL || draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_prev_cursor_x, draw_prev_cursor_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
				if (mirror_x != draw_prev_cursor_x || mirror_y != draw_prev_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &draw_mirror_original_tiles[1], sizeof(u16));
				}
			}
			
			// Restore diagonal mirror
			if (draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_prev_cursor_x, draw_prev_cursor_y, &mirror_x, &mirror_y, MIRROR_BOTH);
				if (mirror_x != draw_prev_cursor_x || mirror_y != draw_prev_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &draw_mirror_original_tiles[2], sizeof(u16));
				}
			}
		}
	}
	
	// Calculate current map position and save original tiles if cursor moved
	int map_pos = draw_cursor_y * 32 + draw_cursor_x;
	if (draw_prev_cursor_x != draw_cursor_x || draw_prev_cursor_y != draw_cursor_y) {
		// Save primary position original tile
		memcpy(&draw_original_tile, layer_ptr + map_pos, sizeof(u16));
		
		// Save mirror positions original tiles
		if (draw_mirror_mode != MIRROR_NONE) {
			// Save horizontal mirror original
			if (draw_mirror_mode == MIRROR_HORIZONTAL || draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
				if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&draw_mirror_original_tiles[0], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
			
			// Save vertical mirror original
			if (draw_mirror_mode == MIRROR_VERTICAL || draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
				if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&draw_mirror_original_tiles[1], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
			
			// Save diagonal mirror original
			if (draw_mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_BOTH);
				if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&draw_mirror_original_tiles[2], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
		}
		
		draw_prev_cursor_x = draw_cursor_x;
		draw_prev_cursor_y = draw_cursor_y;
	}
	
	// Create cursor tile based on blink state
	u16 cursor_tile;
	if (draw_cursor_blink < DRAW_CURSOR_BLINK_SPEED) {
		// Show selected tile with current rotation applied
		cursor_tile = draw_selected_tile;
		switch (draw_tile_rotation) {
			case TILE_ROT_0:
				// No modification needed
				break;
			case TILE_ROT_180:
				// Apply horizontal and vertical flip bits
				cursor_tile |= (1 << 10) | (1 << 11);
				break;
		}
	} else {
		cursor_tile = draw_original_tile; // Show original tile during blink-off
	}
	
	// Render cursor at primary position (temporarily)
	memcpy(layer_ptr + map_pos, &cursor_tile, sizeof(u16));
	
	// Render mirrored cursors based on mirror mode
	if (draw_mirror_mode != MIRROR_NONE) {
		// Calculate and render horizontal mirror
		if (draw_mirror_mode == MIRROR_HORIZONTAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (draw_cursor_blink < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : draw_mirror_original_tiles[0];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
		
		// Calculate and render vertical mirror
		if (draw_mirror_mode == MIRROR_VERTICAL || draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (draw_cursor_blink < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : draw_mirror_original_tiles[1];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
		
		// Calculate and render both mirrors (diagonal mirror)
		if (draw_mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(draw_cursor_x, draw_cursor_y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != draw_cursor_x || mirror_y != draw_cursor_y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (draw_cursor_blink < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : draw_mirror_original_tiles[2];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
	}
}