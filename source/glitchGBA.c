/*
	glitchGBA
	by meru
	
	github: merumerutho
	twitter: merumerutho
	website: meru.cloud
	
	merutochan [[at]] gmail [[dot]] com
	
	credits to: 
	- libgba devs, 
	- coranac for its helpful manual, 
	- infu_av for his precious advices and inspiration, 
	- NO_CARRIER for inpsiration
*/

#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MAP_ADDRESS0 28
#define MAP_ADDRESS1 29
#define MAP_ADDRESS2 30
#define MAP_ADDRESS3 31

#define MAP_LAYER0 &(MAP[MAP_ADDRESS0][0])
#define MAP_LAYER1 &(MAP[MAP_ADDRESS1][0])
#define MAP_LAYER2 &(MAP[MAP_ADDRESS2][0])
#define MAP_LAYER3 &(MAP[MAP_ADDRESS3][0])
#define REG_BG_AFFINE ((BGAffineDest *)(REG_BASE+0x0000))

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define FRAME_COUNT_SCROLLING_MODE 30

#define NUM_LAYERS 4

#define EMULATOR_MODE 0
#define CONSOLE_MODE 1

#include "graphics.h"

// --- global variables --- 
// VSYNC flag
bool vsync = TRUE;

// Layer positions
// Positions
int x_pos[NUM_LAYERS] = {0, 1, 2, 3};
int y_pos[NUM_LAYERS] = {0, 1, 2, 3};

// Layer speeds
int x_speed[NUM_LAYERS] = {0, 1, 2, 3};
int y_speed[NUM_LAYERS] = {0, 1, 2, 3};

// currently-selected tile
int currentTiles = 0;

// currently-selected bg level
int bglevel = 0;

// Are we paused?
bool paused = FALSE;

// Shall we change palette dynamically?
bool paletteChange = FALSE;

// Apply parallax effect flag
bool parallax = FALSE;

// Remap flag (moves tilesets around dynamically, heavy on CPU)
bool remap = FALSE;

// remap background flag (moves tilesets also for layer0, heavy on CPU)
bool bg_remap = FALSE;

// chaos randomizer flag (generates overall chaos)
bool chaos = FALSE;

// frame scrolling mode flag
bool frame_scroll_mode = FALSE;

// device mode (console vs. emulator) flag
bool device_mode = CONSOLE_MODE;

// Frame counter
int frameCounter=0;

// moveRate value (usage depends on hrw/emu)
int moveRateX[NUM_LAYERS] = {0,0,0,0};
int moveRateY[NUM_LAYERS] = {0,0,0,0};

// Rotation mode
bool rotationMode = FALSE;

// Scale in Rotation mode
int scale2 = 1<<8;
int scale3 = 1<<8;
// Theta Speed
int BG2_angular_speed = 0;
int BG3_angular_speed = 0;


// BG Affine Structures (for rotoscaling)
BGAffineSource BG_AFF_S2 = {0, 0, 0, 0, 1<<8, 1<<8, 0x0000};
BGAffineDest BG_AFF_D2;
BGAffineSource BG_AFF_S3 = {0, 0, 0, 0, 1<<8, 1<<8, 0x0000};
BGAffineDest BG_AFF_D3;
	

//

static inline void init_map();
static inline void init_tile();
static void init();
static void updateScreenMovement();
static void updateRemap();
static void updateFrameScroll();
static void updatePalette();

int main();

//

static inline void init_map()
{
	// Reset glitch on tileset
	memcpy(MAP_LAYER0, mapsPointer[currentTiles], MapsLen[currentTiles]);
	memcpy(MAP_LAYER1 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	memcpy(MAP_LAYER2 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
	memcpy(MAP_LAYER3 + (rand()%0xFF), tilesPointer[0]+(rand()%(TilesLen[0]-MapsLen[0])), 0x10);
}

static inline void init_tile()
{
	memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
	memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
	init_map();
}

// Initialization/Reset function
static void init() 
{	
	currentTiles = 0;
	memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
	memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
	
	// Reset map
	init_map();
	bglevel = 0;
	
	for (int i = 0; i < NUM_LAYERS; i++)
	{
		x_speed[i] = 0;
		y_speed[i] = 0;
	}
	
	// eugh brother...
	chaos = FALSE;
	bg_remap = FALSE;
	remap = FALSE;
	parallax = FALSE;
	device_mode = CONSOLE_MODE;
	frame_scroll_mode = FALSE;
	frameCounter=0;
	paused = FALSE;
	vsync = TRUE;
	scale2 = 1<<8;
	scale3 = 1<<8;
	BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
	BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
	BG_AFF_S2.theta = BG_AFF_S3.theta = 0;
	BG2_angular_speed = BG3_angular_speed = 0;
	BG_AFF_S2.x=0;
	BG_AFF_S2.y=0;
	BG_AFF_S3.x=0;
	BG_AFF_S3.y=0;
	
	return;
}

static void updateScreenMovement()
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
			moveRateX[i] = (device_mode == CONSOLE_MODE) ? x_speed[i] : x_pos[i];
			moveRateY[i] = (device_mode == CONSOLE_MODE) ? y_speed[i] : y_pos[i];
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
				BG_AFF_S2.theta += BG2_angular_speed;
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
	return;
}

static void updateRemap()
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

static void updatePalette()
{
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

static void updateFrameScroll()
{
	// Frame-scrolling mode
	if(frame_scroll_mode && frameCounter == FRAME_COUNT_SCROLLING_MODE)
	{
		// Reset frame counter
		frameCounter = 0;
		// Increase current tile (wrap to 0)
		currentTiles = (currentTiles < (NTILES-1)) ? (currentTiles+1) : 0;
		// Swap palette unless palette change flag is enabled
		if(!paletteChange)
		{
			memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
		}
		// Swap base tile and background mapping, but not layers 1,2,3
		memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
		memcpy(&MAP[MAP_ADDRESS0][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
	}
	else if (frameCounter == FRAME_COUNT_SCROLLING_MODE)
	{
		frameCounter = 0;
	}
}

int main() 
{
	// Multikey checker flag
	bool multikey = FALSE;
	
	// IO variables
	int keys_held = 0, keys_released = 0;
	
	// your friendly dummy i
	int i;
	
	// Buffer for palette memory address
	unsigned short * paletteAddress;	
	
	// Enable interrupt VBLANK
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	// Randomize
	scanKeys();
	srand(keysHeld()+1);
	
	// BACKGROUND
	init_tile();
	
	// Place map at random positions
	REG_BG0HOFS = rand() % SCREEN_WIDTH;
	REG_BG0VOFS = rand() % SCREEN_HEIGHT;
	REG_BG1HOFS = rand() % SCREEN_WIDTH;
	REG_BG1VOFS = rand() % SCREEN_HEIGHT;
	REG_BG2HOFS = rand() % SCREEN_WIDTH;
	REG_BG2VOFS = rand() % SCREEN_HEIGHT;
	REG_BG3HOFS = rand() % SCREEN_WIDTH;
	REG_BG3VOFS = rand() % SCREEN_HEIGHT;
	
	// For RotoScale mode as well
	BG_AFF_S2.x = rand() % SCREEN_WIDTH;
	BG_AFF_S2.y = rand() % SCREEN_HEIGHT;
	BG_AFF_S3.x = rand() % SCREEN_WIDTH;
	BG_AFF_S3.y = rand() % SCREEN_HEIGHT;
	
	// Set up registers
	// REGULAR BG
	REG_BG0CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS0) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(3);
	REG_BG1CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS1) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(2);
	REG_BG2CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS2) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(1) | BG_WRAP;
	REG_BG3CNT = TILE_BASE(0) | SCREEN_BASE(MAP_ADDRESS3) | BG_256_COLOR | BG_SIZE(0) | BG_PRIORITY(0) | BG_WRAP;
	
	REG_DISPCNT = (rotationMode << 1) | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
	
	do {
		// Wait for vsync interrupt (60fps lock)
		if(vsync)
		{
			VBlankIntrWait();
		}
		
		frameCounter++;
		
		/* 
			UPDATE DRAW SECTION
		*/
		updateScreenMovement();
		updateRemap();
		updateFrameScroll();
		updatePalette();
		
		// ---
		
		// Check current key status
		scanKeys();
		// Get Pressed Keys
		keys_held = keysHeld();
		// Get Released Keys
		keys_released = keysUp();
		
		// Re-set value of multikey when no keys are pressed
		if ((!keys_held) && (!keys_released))
		{
			multikey = FALSE;
		}
		
		/* 
			CONTROLS SECTION
			Singlekeys
		*/
		if (!keys_held && multikey == FALSE) 
		{	
			// Pause / resume
			switch(keys_released)
			{
				// Pause / unpause
				case KEY_A:
				{
					paused = !paused;
					break;
				}
				// Set Vsync 
				case KEY_B:
				{
					vsync = !vsync;
					break;
				}
				// Set horizontal speed
				case KEY_LEFT:
				{
					x_speed[bglevel]--;
					break;
				}
				case KEY_RIGHT:
				{
					x_speed[bglevel]++;
					break;
				}
				// Set vertical speed
				case KEY_UP:
				{
					y_speed[bglevel]++;
					break;
				}
				case KEY_DOWN:
				{
					y_speed[bglevel]--;
					break;
				}
				// Enable/disable palette change
				case KEY_START:
				{
					paletteChange = !paletteChange;
					break;
				}
				// Enable/disable parallax
				case KEY_SELECT:
				{
					parallax = !parallax;
					break;
				}
				// Swap to right tileset map 
				case KEY_R:
				{
					currentTiles = (currentTiles < (NTILES - 1)) ? (currentTiles+1) : 0;
					// Remap
					memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
					memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
					init_map();
					break;
				}
				// Swap to left tileset map
				case KEY_L:
				{
					currentTiles = (currentTiles > 0) ? (currentTiles-1) : (NTILES - 1);
					memcpy(BG_PALETTE, palsPointer[currentTiles], PalsLen[currentTiles]);
					memcpy(&MAP[0][0], tilesPointer[currentTiles], TilesLen[currentTiles]);
					init_map();
					break;
				}
				default:
				break;
			}
				
		}
		
		/*
			CONTROLS SECTION
			Multikeys
		*/
		
		// Reset status
		if (keys_held == (KEY_A + KEY_B + KEY_START + KEY_SELECT))
		{
			multikey = TRUE;
			init();	
		}
		// Set speeds to 0 without resetting
		else if ((keys_held & KEY_A) && (keys_released & KEY_B))
		{
			multikey = TRUE;
			for (i=0;i<NUM_LAYERS;i++)
			{
				x_speed[i] = 0;
				y_speed[i] = 0;
			}
		}
		// Enable / Disable selected background
		else if ((keys_held & KEY_START) && (keys_released & KEY_B))
		{
			multikey = TRUE;
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
		else if ((keys_held & KEY_START) && (keys_released & KEY_A))
		{
			multikey = TRUE;
			remap = !remap;
		}
		// Turn background remap on/off		
		else if ((keys_held & KEY_SELECT) && (keys_released & KEY_A))
		{
			multikey = TRUE;
			bg_remap = !bg_remap;
		}	
		// Turn device_mode mod on/off
		else if ((keys_held & KEY_SELECT) && (keys_released & KEY_B))
		{
			multikey = TRUE;
			device_mode = !device_mode;
		}	
		// Increase level of background
		else if ((keys_held & KEY_B) && (keys_released & KEY_UP))
		{
			multikey = TRUE;
			bglevel = (bglevel < (NUM_LAYERS-1)) ? (bglevel + 1) : 0;
		}	
		// Decrease level of background
		else if ((keys_held & KEY_B) && (keys_released & KEY_DOWN))
		{
			multikey = TRUE;
			bglevel = (bglevel > 0) ? bglevel-1 : (NUM_LAYERS-1);
		}	
		// Enable / Disable chaos mode
		else if (keys_held == (KEY_START + KEY_SELECT))
		{
			multikey = TRUE;
			// Invert chaos
			chaos = !chaos;
			// Vsync set to opposite of chaos flag
			vsync = !chaos;
		}
		// In rotation mode, rotate right
		else if ((keys_held == (KEY_R + KEY_A)))
		{
			multikey = TRUE;
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
		else if ((keys_held == (KEY_L + KEY_A)))
		{
			multikey = TRUE;
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
		else if ((keys_held == (KEY_R + KEY_B)))
		{
			multikey = TRUE;
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
		else if ((keys_held == (KEY_L + KEY_B)))
		{
			multikey = TRUE;
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
		else if ((keys_held & KEY_R) && (keys_released & KEY_RIGHT))
		{
			multikey = TRUE;
			frame_scroll_mode = !frame_scroll_mode;
		}	
		// Switch rotation mode - normal mode
		else if ((keys_held & KEY_L) && (keys_released & KEY_R)) 
		{
			multikey = TRUE;
			rotationMode=!rotationMode;
			if (rotationMode)
			{
				REG_DISPCNT = MODE_2 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
				memcpy(&MAP[MAP_ADDRESS2][0], mapsPointer[currentTiles], MapsLen[currentTiles]);
				memcpy(&MAP[MAP_ADDRESS3][0], tilesPointer[currentTiles]+(rand()%(TilesLen[currentTiles])), MapsLen[currentTiles]);
			} 
			else 
			{
				REG_DISPCNT = BG0_ON | BG1_ON | BG2_ON | BG3_ON;
				init_map();
				bglevel = 0;
			}
		}
		
	} while( 1 );
}