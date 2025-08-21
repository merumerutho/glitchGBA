#ifndef INPUT_H
#define INPUT_H

#include <gba.h>

// Input handling structure
typedef struct {
	int keys_held;
	int keys_released;
	bool multikey;
} InputState;

// Function declarations
void input_init(void);
void input_update(InputState* input);
void input_handle_single_keys(InputState* input);
void input_handle_multi_keys(InputState* input);

#endif // INPUT_H