
//{{BLOCK(pyramid)

//======================================================================
//
//	pyramid, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 264 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x32 
//	Total size: 512 + 16896 + 2048 = 19456
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_PYRAMID_H
#define GRIT_PYRAMID_H

#define pyramidTilesLen 16896
extern const unsigned int pyramidTiles[4224];

#define pyramidMapLen 2048
extern const unsigned short pyramidMap[1024];

#define pyramidPalLen 512
extern const unsigned short pyramidPal[256];

#endif // GRIT_PYRAMID_H

//}}BLOCK(pyramid)
