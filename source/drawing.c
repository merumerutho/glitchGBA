#include "drawing.h"
#include "display.h"
#include "graphics.h"
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state
extern DrawingSystem drawing;
extern int currentTiles;

// Forward declarations
extern void init_map(void);

void drawing_init(void)
{
	// Initialize drawing cursor to center of screen
	drawing.cursor.x = DRAW_CURSOR_MAX_X / 2;
	drawing.cursor.y = DRAW_CURSOR_MAX_Y / 2;
	drawing.cursor.prev_x = -1;
	drawing.cursor.prev_y = -1;
	drawing.cursor.blink_counter = 0;
	
	// Initialize tile state
	drawing.tile.selected_tile = 0;
	drawing.tile.active_layer = 0;
	drawing.tile.rotation = TILE_NO_FLIP;
	drawing.tile.mirror_mode = MIRROR_NONE;
	
	// Initialize input state
	drawing.input.repeat_timer = 0;
	drawing.input.last_held_keys = 0;
	
	// Initialize backup tiles
	drawing.backup.primary = 0;
	drawing.backup.mirrors[0] = drawing.backup.mirrors[1] = drawing.backup.mirrors[2] = 0;
}

void drawing_update(void)
{
	if (!drawing.enabled) return;
	
	// Update cursor blink animation
	drawing.cursor.blink_counter++;
	if (drawing.cursor.blink_counter >= DRAW_CURSOR_BLINK_SPEED * 2) {
		drawing.cursor.blink_counter = 0;
	}
	
	// Render cursor
	drawing_render_cursor();
}

void drawing_move_cursor(int dx, int dy)
{
	if (!drawing.enabled) return;
	
	// Update cursor position with bounds checking
	drawing.cursor.x += dx;
	drawing.cursor.y += dy;
	
	// Wrap around screen edges
	drawing.cursor.x = (drawing.cursor.x < 0) ? DRAW_CURSOR_MAX_X : (drawing.cursor.x > DRAW_CURSOR_MAX_X) ? 0 : drawing.cursor.x;
	drawing.cursor.y = (drawing.cursor.y < 0) ? DRAW_CURSOR_MAX_Y : (drawing.cursor.y > DRAW_CURSOR_MAX_Y) ? 0 : drawing.cursor.y;
}

void drawing_place_tile(void)
{
	if (!drawing.enabled) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (drawing.tile.active_layer) {
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
	
	// Apply flip bits using GBA hardware flip functionality
	u16 tile_entry = drawing.tile.selected_tile |
		((drawing.tile.rotation == TILE_FLIP_H || drawing.tile.rotation == TILE_FLIP_HV) << 10) |
		((drawing.tile.rotation == TILE_FLIP_V || drawing.tile.rotation == TILE_FLIP_HV) << 11) ;
	
	// Place tile at primary cursor position
	int map_pos = drawing.cursor.y * 32 + drawing.cursor.x;
	memcpy(layer_ptr + map_pos, &tile_entry, sizeof(u16));
	
	// Place tiles at mirrored positions based on mirror mode
	if (drawing.tile.mirror_mode != MIRROR_NONE) {
		// Place tile at horizontal mirror
		if (drawing.tile.mirror_mode == MIRROR_HORIZONTAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
		
		// Place tile at vertical mirror
		if (drawing.tile.mirror_mode == MIRROR_VERTICAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
		
		// Place tile at both mirrors (diagonal mirror)
		if (drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate placement
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &tile_entry, sizeof(u16));
			}
		}
	}
}

void drawing_erase_tile(void)
{
	if (!drawing.enabled) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (drawing.tile.active_layer) {
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
	int map_pos = drawing.cursor.y * 32 + drawing.cursor.x;
	memcpy(layer_ptr + map_pos, &zero_tile, sizeof(u16));
	
	// Erase tiles at mirrored positions based on mirror mode
	if (drawing.tile.mirror_mode != MIRROR_NONE) {
		// Erase tile at horizontal mirror
		if (drawing.tile.mirror_mode == MIRROR_HORIZONTAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
		
		// Erase tile at vertical mirror
		if (drawing.tile.mirror_mode == MIRROR_VERTICAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
		
		// Erase tile at both mirrors (diagonal mirror)
		if (drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) { // Avoid duplicate erasure
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				memcpy(layer_ptr + mirror_map_pos, &zero_tile, sizeof(u16));
			}
		}
	}
}

void drawing_clear_layer(void)
{
	if (!drawing.enabled) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (drawing.tile.active_layer) {
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
	drawing.backup.primary = 0;
}

void drawing_next_tile(void)
{
	if (!drawing.enabled) return;
	
	// Get the number of tiles in current tileset
	int max_tiles = TilesLen[currentTiles] / 64; // Each tile is 64 bytes (8x8x8bit)
	
	drawing.tile.selected_tile = (++drawing.tile.selected_tile >= max_tiles) ? 0 : drawing.tile.selected_tile;
}

void drawing_prev_tile(void)
{
	if (!drawing.enabled) return;
	
	// Get the number of tiles in current tileset
	int max_tiles = TilesLen[currentTiles] / 64; // Each tile is 64 bytes (8x8x8bit)
	
	drawing.tile.selected_tile = (--drawing.tile.selected_tile < 0) ? (max_tiles - 1) : drawing.tile.selected_tile;
}

void drawing_next_layer(void)
{
	if (!drawing.enabled) return;
	
	drawing.tile.active_layer = (++drawing.tile.active_layer > 3) ? 0 : drawing.tile.active_layer;
}

void drawing_toggle_rotation(void)
{
	if (!drawing.enabled) return;
	
	// Cycle through flip modes: No Flip -> H Flip -> V Flip -> HV Flip -> No Flip
	switch (drawing.tile.rotation) {
		case TILE_NO_FLIP:
			drawing.tile.rotation = TILE_FLIP_H;
			break;
		case TILE_FLIP_H:
			drawing.tile.rotation = TILE_FLIP_V;
			break;
		case TILE_FLIP_V:
			drawing.tile.rotation = TILE_FLIP_HV;
			break;
		case TILE_FLIP_HV:
			drawing.tile.rotation = TILE_NO_FLIP;
			break;
	}
}

void drawing_cycle_mirror_mode(void)
{
	if (!drawing.enabled) return;
	
	// Cycle through mirror modes: None -> Horizontal -> Vertical -> Both -> None
	switch (drawing.tile.mirror_mode) {
		case MIRROR_NONE:
			drawing.tile.mirror_mode = MIRROR_HORIZONTAL;
			break;
		case MIRROR_HORIZONTAL:
			drawing.tile.mirror_mode = MIRROR_VERTICAL;
			break;
		case MIRROR_VERTICAL:
			drawing.tile.mirror_mode = MIRROR_BOTH;
			break;
		case MIRROR_BOTH:
			drawing.tile.mirror_mode = MIRROR_NONE;
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
	if (!drawing.enabled) return;
	
	// Check if the held keys changed
	if (keys_held != drawing.input.last_held_keys) {
		drawing.input.repeat_timer = 0;
		drawing.input.last_held_keys = keys_held;
		return;
	}
	
	// If no keys held, reset timer
	if (keys_held == 0) {
		drawing.input.repeat_timer = 0;
		return;
	}
	
	// Increment repeat timer
	drawing.input.repeat_timer++;
	
	// Check if we should trigger repeat actions
	bool should_repeat = FALSE;
	
	if (drawing.input.repeat_timer == KEY_REPEAT_DELAY) {
		// Initial repeat after delay
		should_repeat = TRUE;
	} else if (drawing.input.repeat_timer > KEY_REPEAT_DELAY && 
			   (drawing.input.repeat_timer - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
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
	if (!drawing.enabled) return;
	
	// Get active layer pointer
	u16 *layer_ptr;
	switch (drawing.tile.active_layer) {
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
	if (drawing.cursor.prev_x != -1 && drawing.cursor.prev_y != -1 && 
		(drawing.cursor.prev_x != drawing.cursor.x || drawing.cursor.prev_y != drawing.cursor.y)) {
		
		// Restore primary position
		int prev_map_pos = drawing.cursor.prev_y * 32 + drawing.cursor.prev_x;
		memcpy(layer_ptr + prev_map_pos, &drawing.backup.primary, sizeof(u16));
		
		// Restore mirrored positions using their saved original tiles
		if (drawing.tile.mirror_mode != MIRROR_NONE) {
			// Restore horizontal mirror
			if (drawing.tile.mirror_mode == MIRROR_HORIZONTAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.prev_x, drawing.cursor.prev_y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
				if (mirror_x != drawing.cursor.prev_x || mirror_y != drawing.cursor.prev_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &drawing.backup.mirrors[0], sizeof(u16));
				}
			}
			
			// Restore vertical mirror  
			if (drawing.tile.mirror_mode == MIRROR_VERTICAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.prev_x, drawing.cursor.prev_y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
				if (mirror_x != drawing.cursor.prev_x || mirror_y != drawing.cursor.prev_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &drawing.backup.mirrors[1], sizeof(u16));
				}
			}
			
			// Restore diagonal mirror
			if (drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.prev_x, drawing.cursor.prev_y, &mirror_x, &mirror_y, MIRROR_BOTH);
				if (mirror_x != drawing.cursor.prev_x || mirror_y != drawing.cursor.prev_y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(layer_ptr + mirror_map_pos, &drawing.backup.mirrors[2], sizeof(u16));
				}
			}
		}
	}
	
	// Calculate current map position and save original tiles if cursor moved
	int map_pos = drawing.cursor.y * 32 + drawing.cursor.x;
	if (drawing.cursor.prev_x != drawing.cursor.x || drawing.cursor.prev_y != drawing.cursor.y) {
		// Save primary position original tile
		memcpy(&drawing.backup.primary, layer_ptr + map_pos, sizeof(u16));
		
		// Save mirror positions original tiles
		if (drawing.tile.mirror_mode != MIRROR_NONE) {
			// Save horizontal mirror original
			if (drawing.tile.mirror_mode == MIRROR_HORIZONTAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
				if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&drawing.backup.mirrors[0], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
			
			// Save vertical mirror original
			if (drawing.tile.mirror_mode == MIRROR_VERTICAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
				if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&drawing.backup.mirrors[1], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
			
			// Save diagonal mirror original
			if (drawing.tile.mirror_mode == MIRROR_BOTH) {
				int mirror_x, mirror_y;
				drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_BOTH);
				if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
					int mirror_map_pos = mirror_y * 32 + mirror_x;
					memcpy(&drawing.backup.mirrors[2], layer_ptr + mirror_map_pos, sizeof(u16));
				}
			}
		}
		
		drawing.cursor.prev_x = drawing.cursor.x;
		drawing.cursor.prev_y = drawing.cursor.y;
	}
	
	// Create cursor tile based on blink state
	u16 cursor_tile;
	if (drawing.cursor.blink_counter < DRAW_CURSOR_BLINK_SPEED) {
		// Show selected tile with current flip mode applied
		cursor_tile = drawing.tile.selected_tile;
		switch (drawing.tile.rotation) {
			case TILE_NO_FLIP:
				// No modification needed
				break;
			case TILE_FLIP_H:
				// Apply horizontal flip bit (bit 10)
				cursor_tile |= (1 << 10);
				break;
			case TILE_FLIP_V:
				// Apply vertical flip bit (bit 11)
				cursor_tile |= (1 << 11);
				break;
			case TILE_FLIP_HV:
				// Apply both horizontal and vertical flip bits
				cursor_tile |= (1 << 10) | (1 << 11);
				break;
		}
	} else {
		cursor_tile = drawing.backup.primary; // Show original tile during blink-off
	}
	
	// Render cursor at primary position (temporarily)
	memcpy(layer_ptr + map_pos, &cursor_tile, sizeof(u16));
	
	// Render mirrored cursors based on mirror mode
	if (drawing.tile.mirror_mode != MIRROR_NONE) {
		// Calculate and render horizontal mirror
		if (drawing.tile.mirror_mode == MIRROR_HORIZONTAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_HORIZONTAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (drawing.cursor.blink_counter < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : drawing.backup.mirrors[0];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
		
		// Calculate and render vertical mirror
		if (drawing.tile.mirror_mode == MIRROR_VERTICAL || drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_VERTICAL);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (drawing.cursor.blink_counter < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : drawing.backup.mirrors[1];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
		
		// Calculate and render both mirrors (diagonal mirror)
		if (drawing.tile.mirror_mode == MIRROR_BOTH) {
			int mirror_x, mirror_y;
			drawing_get_mirror_positions(drawing.cursor.x, drawing.cursor.y, &mirror_x, &mirror_y, MIRROR_BOTH);
			if (mirror_x != drawing.cursor.x || mirror_y != drawing.cursor.y) {
				int mirror_map_pos = mirror_y * 32 + mirror_x;
				u16 mirror_cursor_tile = (drawing.cursor.blink_counter < DRAW_CURSOR_BLINK_SPEED) ? cursor_tile : drawing.backup.mirrors[2];
				memcpy(layer_ptr + mirror_map_pos, &mirror_cursor_tile, sizeof(u16));
			}
		}
	}
}