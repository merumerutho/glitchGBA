#include "input.h"
#include "display.h"
#include "graphics.h"
#include "drawing.h"
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

// Key repeat timing constants (from drawing.h)
#define KEY_REPEAT_DELAY 15     // Initial delay before repeat starts (frames)
#define KEY_REPEAT_RATE 4       // Repeat rate once started (frames between repeats)

// External references to global state
extern bool paused, vsync, parallax, paletteChange, chaos;
extern bool remap, bg_remap, frame_scroll_mode, rotationMode;
extern DrawingSystem drawing;
extern int x_speed[4], y_speed[4];
extern int bglevel, currentTiles;
extern int scale2, scale3, BG2_angular_speed, BG3_angular_speed;
extern BGAffineSource BG_AFF_S2, BG_AFF_S3;
extern AppMode currentMode;

// Forward declaration for reset function
extern void init(void);
extern void cycle_app_mode(void);
extern void drawing_cycle_mirror_mode(void);

// =============================================================================
// ACTION FUNCTIONS - Each key binding maps to one of these functions
// =============================================================================

// Global actions (available in all modes)
void action_cycle_mode(void) { cycle_app_mode(); }
void action_reset_all(void) { init(); }
void action_toggle_chaos(void) { vsync = !(chaos = !chaos); }

// Normal mode actions
void action_toggle_pause(void) { paused = !paused; }
void action_toggle_vsync(void) { vsync = !vsync; }
void action_toggle_parallax(void) { parallax = !parallax; }
void action_toggle_palette_change(void) { paletteChange = !paletteChange; }
void action_toggle_remap(void) { remap = !remap; }
void action_toggle_bg_remap(void) { bg_remap = !bg_remap; }
void action_toggle_frame_scroll(void) { frame_scroll_mode = !frame_scroll_mode; }

void action_move_left(void) { x_speed[bglevel]--; }
void action_move_right(void) { x_speed[bglevel]++; }
void action_move_up(void) { y_speed[bglevel]++; }
void action_move_down(void) { y_speed[bglevel]--; }

void action_tileset_next(void) {
    currentTiles = (currentTiles < (NTILES - 1)) ? (currentTiles+1) : 0;
    drawing.tile.selected_tile = 0;
    dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
}

void action_tileset_prev(void) {
    currentTiles = (currentTiles > 0) ? (currentTiles-1) : (NTILES - 1);
    drawing.tile.selected_tile = 0;
    dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
    dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
}

void action_layer_next(void) { bglevel = (bglevel < (NUM_LAYERS-1)) ? (bglevel + 1) : 0; }
void action_layer_prev(void) { bglevel = (bglevel > 0) ? bglevel-1 : (NUM_LAYERS-1); }

void action_stop_all_layers(void) {
    for (int i=0; i<NUM_LAYERS; i++) {
        x_speed[i] = 0;
        y_speed[i] = 0;
    }
}

void action_toggle_bg_layer(void) {
    if (bglevel==0) REG_DISPCNT^=BG0_ON;
    else if (bglevel==1) REG_DISPCNT^=BG1_ON;
    else if (bglevel==2) REG_DISPCNT^=BG2_ON;
    else if (bglevel==3) REG_DISPCNT^=BG3_ON;
}

void action_increment_tile_indices(void) { drawing_increment_tiles(); }
void action_decrement_tile_indices(void) { drawing_decrement_tiles(); }

// Rotation mode actions
void action_rotate_right(void) {
    ((bglevel % 2) == 0) ? (BG2_angular_speed -= 1) : (BG3_angular_speed -= 1);
}

void action_rotate_left(void) {
    ((bglevel % 2) == 0) ? (BG2_angular_speed += 1) : (BG3_angular_speed += 1);
}

void action_zoom_in(void) {
    if ((bglevel % 2) == 0) {
        scale2 += 1;
        BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
    } else {
        scale3 += 1;
        BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
    }
}

void action_zoom_out(void) {
    if ((bglevel % 2) == 0) {
        scale2 -= 1;
        BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
    } else {
        scale3 -= 1;
        BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
    }
}

// Drawing mode actions
void action_drawing_place_tile(void) { drawing_place_tile(); }
void action_drawing_erase_tile(void) { drawing_erase_tile(); }
void action_drawing_clear_layer(void) { drawing_clear_layer(); }
void action_drawing_next_tile(void) { drawing_next_tile(); }
void action_drawing_prev_tile(void) { drawing_prev_tile(); }
void action_drawing_next_layer(void) { drawing_next_layer(); }
void action_drawing_toggle_rotation(void) { drawing_toggle_rotation(); }
void action_drawing_cycle_mirror(void) { drawing_cycle_mirror_mode(); }

void action_drawing_move_left(void) { drawing_move_cursor(-1, 0); }
void action_drawing_move_right(void) { drawing_move_cursor(1, 0); }
void action_drawing_move_up(void) { drawing_move_cursor(0, -1); }
void action_drawing_move_down(void) { drawing_move_cursor(0, 1); }

// =============================================================================
// KEY BINDINGS TABLE - Central registry of all input mappings
// =============================================================================

static const KeyBinding key_bindings[] = {
    // =========================================================================
    // GLOBAL BINDINGS (available in all modes)
    // =========================================================================
    {KEY_L, KEY_R, false, true, MODE_NORMAL, action_cycle_mode, "Cycle app modes"},
    // Note: Complex multi-key combinations need special handling - will implement later
    // {KEY_A | KEY_B | KEY_START, KEY_SELECT, false, true, MODE_NORMAL, action_reset_all, "Reset all settings"},
    {KEY_START, KEY_SELECT, false, true, MODE_NORMAL, action_toggle_chaos, "Toggle chaos mode"},

    // =========================================================================
    // NORMAL MODE BINDINGS
    // =========================================================================
    // Single key actions
    {0, KEY_A, false, false, MODE_NORMAL, action_toggle_pause, "Toggle pause"},
    {0, KEY_B, false, false, MODE_NORMAL, action_toggle_vsync, "Toggle vsync"},
    {0, KEY_SELECT, false, false, MODE_NORMAL, action_toggle_parallax, "Toggle parallax"},
    {0, KEY_START, false, false, MODE_NORMAL, action_toggle_palette_change, "Toggle palette change"},
    {0, KEY_LEFT, false, false, MODE_NORMAL, action_move_left, "Move left"},
    {0, KEY_RIGHT, false, false, MODE_NORMAL, action_move_right, "Move right"},
    {0, KEY_UP, false, false, MODE_NORMAL, action_move_up, "Move up"},
    {0, KEY_DOWN, false, false, MODE_NORMAL, action_move_down, "Move down"},
    {0, KEY_R, false, false, MODE_NORMAL, action_tileset_next, "Next tileset"},
    //{0, KEY_L, false, false, MODE_NORMAL, action_tileset_prev, "Previous tileset"},
    
    // Multi-key actions
    {KEY_A, KEY_B, false, false, MODE_NORMAL, action_stop_all_layers, "Stop all layers"},
    {KEY_START, KEY_B, false, false, MODE_NORMAL, action_toggle_bg_layer, "Toggle BG layer"},
    {KEY_START, KEY_A, false, false, MODE_NORMAL, action_toggle_remap, "Toggle remap"},
    {KEY_SELECT, KEY_A, false, false, MODE_NORMAL, action_toggle_bg_remap, "Toggle BG remap"},
    {KEY_B, KEY_UP, false, false, MODE_NORMAL, action_layer_next, "Next layer"},
    {KEY_B, KEY_DOWN, false, false, MODE_NORMAL, action_layer_prev, "Previous layer"},
    {KEY_R, KEY_RIGHT, false, false, MODE_NORMAL, action_toggle_frame_scroll, "Toggle frame scroll"},
    
    // Continuous actions
    {KEY_START, KEY_L, true, true, 0, action_decrement_tile_indices, "Decrement tile indices"},
    {KEY_START, KEY_R, true, true, 0, action_increment_tile_indices, "Increment tile indices"},

    // =========================================================================
    // ROTATION MODE BINDINGS
    // =========================================================================
    {KEY_R, KEY_A, true, false, MODE_ROTATION, action_rotate_right, "Rotate right"},
    {KEY_L, KEY_A, true, false, MODE_ROTATION, action_rotate_left, "Rotate left"},
    {KEY_R, KEY_B, true, false, MODE_ROTATION, action_zoom_in, "Zoom in"},
    {KEY_L, KEY_B, true, false, MODE_ROTATION, action_zoom_out, "Zoom out"},

    // =========================================================================
    // DRAWING MODE BINDINGS
    // =========================================================================
    // Note: Basic single-key actions (A, B, L, R, directions) and continuous actions 
    // are handled by drawing_handle_continuous_input() - only multi-key actions here
    
    // Multi-key actions
    {KEY_SELECT, KEY_B, false, false, MODE_DRAWING, action_drawing_toggle_rotation, "Toggle rotation"},
    {KEY_START, KEY_B, false, false, MODE_DRAWING, action_drawing_cycle_mirror, "Cycle mirror mode"},
    {KEY_START, KEY_SELECT, false, false, MODE_DRAWING, action_drawing_clear_layer, "Clear layer"},
    {KEY_B, KEY_UP, false, false, MODE_DRAWING, action_drawing_next_layer, "Next layer"},
};

static const int NUM_KEY_BINDINGS = sizeof(key_bindings) / sizeof(KeyBinding);

void input_init(void)
{
	// Initialize repeat timers to zero
	// The new InputState structure will handle this automatically
}

// =============================================================================
// CENTRALIZED INPUT PROCESSING SYSTEM
// =============================================================================

bool input_check_combination(InputState* input, const KeyBinding* binding)
{
	if (binding->modifier_key == 0) {
		// Single key binding - check if action key was released while no other keys held
		return (input->keys_released == binding->action_key && input->keys_held == 0);
	} else {
		// Multi-key binding - check if modifier held and action key released
		return ((input->keys_held & binding->modifier_key) && (input->keys_released & binding->action_key));
	}
}

void input_handle_continuous_action(InputState* input, const KeyBinding* binding, int binding_index)
{
	// Ensure we don't exceed timer array bounds
	if (binding_index >= MAX_REPEAT_TIMERS) return;
	
	int current_combination = binding->modifier_key | binding->action_key;
	
	// Check if this combination is currently active
	bool combination_active = false;
	if (binding->modifier_key == 0) {
		combination_active = (input->keys_held & binding->action_key);
	} else {
		combination_active = ((input->keys_held & binding->modifier_key) && (input->keys_held & binding->action_key));
	}
	
	if (!combination_active) {
		input->repeat_timers[binding_index] = 0;
		return;
	}
	
	// Check if the combination changed
	if (input->last_combination != current_combination) {
		input->repeat_timers[binding_index] = 0;
		input->last_combination = current_combination;
		return;
	}
	
	// Increment repeat timer
	input->repeat_timers[binding_index]++;
	
	// Check if we should trigger the action
	bool should_repeat = false;
	if (input->repeat_timers[binding_index] == KEY_REPEAT_DELAY) {
		should_repeat = true;
	} else if (input->repeat_timers[binding_index] > KEY_REPEAT_DELAY && 
			   (input->repeat_timers[binding_index] - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE == 0) {
		should_repeat = true;
	}
	
	if (should_repeat && binding->action != NULL) {
		binding->action();
		input->multikey = true;  // Set multikey flag for legacy compatibility
	}
}

void input_handle_single_action(InputState* input, const KeyBinding* binding)
{
	if (binding->action != NULL) {
		binding->action();
		input->multikey = true;  // Set multikey flag for legacy compatibility
	}
}

void input_process(InputState* input)
{
	// Handle drawing mode's complex continuous input first
	if (currentMode == MODE_DRAWING) {
		drawing_handle_continuous_input(input->keys_held);
	}
	
	// Clear multikey flag at start of processing
	input->multikey = false;
	
	// Process all key bindings
	for (int i = 0; i < NUM_KEY_BINDINGS; i++) {
		const KeyBinding* binding = &key_bindings[i];
		
		// Check if binding applies to current mode
		if (!binding->all_modes && binding->specific_mode != currentMode) {
			continue;
		}
		
		// Check if combination is active
		bool combination_matches = false;
		
		if (binding->continuous) {
			// For continuous actions, check if keys are currently held
			if (binding->modifier_key == 0) {
				combination_matches = (input->keys_held & binding->action_key);
			} else {
				combination_matches = ((input->keys_held & binding->modifier_key) && (input->keys_held & binding->action_key));
			}
			
			if (combination_matches) {
				input_handle_continuous_action(input, binding, i % MAX_REPEAT_TIMERS);
			}
		} else {
			// For single actions, use the standard combination check
			combination_matches = input_check_combination(input, binding);
			
			if (combination_matches) {
				input_handle_single_action(input, binding);
			}
		}
	}
}


void input_update(InputState* input)
{
	// Check current key status
	scanKeys();
	
	// Get current key states
	input->keys_held = keysHeld();
	input->keys_released = keysUp();
	input->keys_pressed = keysDown();  // Keys just pressed this frame
	
	// Initialize repeat timers and combination tracking on first call
	static bool initialized = false;
	if (!initialized) {
		memset(input->repeat_timers, 0, sizeof(input->repeat_timers));
		input->last_combination = 0;
		initialized = true;
	}
	
	// Re-set value of multikey when no keys are pressed (legacy compatibility)
	if ((!input->keys_held) && (!input->keys_released)) {
		input->multikey = FALSE;
	}
}