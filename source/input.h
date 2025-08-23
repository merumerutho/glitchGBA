#ifndef INPUT_H
#define INPUT_H

#include <gba.h>

#define MAX_KEY_COMBINATIONS 32
#define MAX_REPEAT_TIMERS 16

// Forward declaration for AppMode
typedef enum {
    MODE_NORMAL,
    MODE_ROTATION,
    MODE_DRAWING
} AppMode;

// Unified input state structure
typedef struct {
    int keys_held;
    int keys_released;
    int keys_pressed;           // Keys just pressed this frame
    bool multikey;              // Legacy compatibility
    
    // Unified repeat timing for all combinations
    int repeat_timers[MAX_REPEAT_TIMERS];
    int last_combination;       // Track which combination was active last frame
} InputState;

// Key binding structure for centralized input handling
typedef struct {
    int modifier_key;           // Key that must be held (0 = none for single key)
    int action_key;            // Key that triggers action
    bool continuous;           // Whether it repeats when held
    bool all_modes;           // Available in all modes
    AppMode specific_mode;     // Or specific to this mode (ignored if all_modes = true)
    void (*action)(void);      // Function to call
    const char* description;   // For debugging/documentation
} KeyBinding;

// Function declarations
void input_init(void);
void input_update(InputState* input);
void input_process(InputState* input);
bool input_check_combination(InputState* input, const KeyBinding* binding);
void input_handle_continuous_action(InputState* input, const KeyBinding* binding, int binding_index);
void input_handle_single_action(InputState* input, const KeyBinding* binding);

#endif // INPUT_H