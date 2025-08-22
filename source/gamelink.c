#include "gamelink.h"
#include "graphics.h"
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

// External references to global state (from main)
extern bool paused, vsync, parallax, paletteChange, chaos;
extern bool remap, bg_remap, frame_scroll_mode, rotationMode;
extern int x_speed[4], y_speed[4];
extern int bglevel, currentTiles;
extern int scale2, scale3, BG2_angular_speed, BG3_angular_speed;
extern BGAffineSource BG_AFF_S2, BG_AFF_S3;

// Forward declarations
extern void init(void);
extern void init_map(void);

// Game Link Cable state variables
u8 gamelink_cmd_buffer[GAMELINK_CMD_BUFFER_SIZE];
u8 gamelink_cmd_head = 0;
u8 gamelink_cmd_tail = 0;
bool gamelink_enabled = TRUE;

// Initialize Game Link Cable for receiving commands
void gamelink_init(void)
{
	// Set up serial communication in 8-bit receive mode
	// Configure as slave (receiver) without external clock
	REG_SIOCNT = SIO_8BIT | SIO_RECV_ONLY | SIO_ENABLE;
	
	// Initialize command buffer
	gamelink_cmd_head = 0;
	gamelink_cmd_tail = 0;
	
	// Clear any pending data
	REG_SIODATA8;
}

// Poll for incoming Game Link data and buffer commands
void gamelink_poll(void)
{
	if (!gamelink_enabled) return;
	
	// Check if serial transfer is complete (data received)
	if (REG_SIOCNT & 0x0008) // Check transfer flag
	{
		u8 received_data = REG_SIODATA8;
		
		// Only buffer valid command bytes (ignore 0xFF which indicates no data)
		if (received_data != GAMELINK_NO_DATA)
		{
			// Add to circular buffer if there's space
			u8 next_head = (gamelink_cmd_head + 1) % GAMELINK_CMD_BUFFER_SIZE;
			if (next_head != gamelink_cmd_tail)
			{
				gamelink_cmd_buffer[gamelink_cmd_head] = received_data;
				gamelink_cmd_head = next_head;
			}
		}
		
		// Clear transfer flag by reading SIODATA8 again
		REG_SIODATA8;
	}
}

// Check if commands are available in buffer
bool gamelink_has_command(void)
{
	return gamelink_cmd_head != gamelink_cmd_tail;
}

// Get next command from buffer
u8 gamelink_get_command(void)
{
	if (!gamelink_has_command()) return GAMELINK_NO_DATA;
	
	u8 command = gamelink_cmd_buffer[gamelink_cmd_tail];
	gamelink_cmd_tail = (gamelink_cmd_tail + 1) % GAMELINK_CMD_BUFFER_SIZE;
	return command;
}

// Process a received Game Link command
void gamelink_process_command(u8 command)
{
	// Game Link command protocol - map bytes to glitchGBA functions
	switch (command)
	{
		// === BASIC CONTROLS ===
		case 0x01: paused = !paused; break;                    // Toggle pause
		case 0x02: vsync = !vsync; break;                      // Toggle vsync
		case 0x03: parallax = !parallax; break;               // Toggle parallax
		case 0x04: paletteChange = !paletteChange; break;      // Toggle palette change
		case 0x05: chaos = !chaos; vsync = !chaos; break;     // Toggle chaos mode
		
		// === LAYER SPEED CONTROLS ===
		case 0x10: x_speed[bglevel]--; break;                 // Decrease X speed
		case 0x11: x_speed[bglevel]++; break;                 // Increase X speed  
		case 0x12: y_speed[bglevel]--; break;                 // Decrease Y speed
		case 0x13: y_speed[bglevel]++; break;                 // Increase Y speed
		case 0x14: // Stop all movement
			for (int i=0; i<4; i++) { x_speed[i] = 0; y_speed[i] = 0; }
			break;
			
		// === LAYER SELECTION ===
		case 0x20: bglevel = (bglevel + 1) % 4; break; // Next layer
		case 0x21: bglevel = (bglevel > 0) ? bglevel-1 : 3; break; // Previous layer
		
		// === TILESET CONTROLS ===
		case 0x30: // Next tileset with reglitch
			currentTiles = (currentTiles < (NTILES-1)) ? (currentTiles+1) : 0;
			dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
			dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
			init_map();
			break;
		case 0x31: // Previous tileset with reglitch
			currentTiles = (currentTiles > 0) ? (currentTiles-1) : (NTILES-1);
			dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
			dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
			init_map();
			break;
			
		// === MODE TOGGLES ===
		case 0x40: remap = !remap; break;                      // Toggle remap
		case 0x41: bg_remap = !bg_remap; break;               // Toggle bg remap
		case 0x42: frame_scroll_mode = !frame_scroll_mode; break; // Toggle auto-scroll
		case 0x43: // Toggle rotation mode
			rotationMode = !rotationMode;
			if (rotationMode) {
				REG_DISPCNT = MODE_2 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
			} else {
				REG_DISPCNT = BG0_ON | BG1_ON | BG2_ON | BG3_ON;
			}
			break;
			
		// === ROTATION CONTROLS (only in rotation mode) ===
		case 0x50: // Rotate current layer left
			if (rotationMode) {
				if ((bglevel % 2) == 0) BG2_angular_speed += 1;
				else BG3_angular_speed += 1;
			}
			break;
		case 0x51: // Rotate current layer right  
			if (rotationMode) {
				if ((bglevel % 2) == 0) BG2_angular_speed -= 1;
				else BG3_angular_speed -= 1;
			}
			break;
		case 0x52: // Zoom in
			if (rotationMode) {
				if ((bglevel % 2) == 0) {
					scale2 += 1; BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
				} else {
					scale3 += 1; BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
				}
			}
			break;
		case 0x53: // Zoom out
			if (rotationMode) {
				if ((bglevel % 2) == 0) {
					scale2 -= 1; BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
				} else {
					scale3 -= 1; BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
				}
			}
			break;
			
		// === SYSTEM CONTROLS ===
		case 0xF0: gamelink_enabled = !gamelink_enabled; break; // Toggle Game Link
		case 0xFF: init(); break;                               // Reset application
		
		default:
			// Unknown command - ignore
			break;
	}
}