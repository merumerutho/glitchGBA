
//{{BLOCK(temple)

//======================================================================
//
//	temple, 256x240@8, 
//	+ palette 256 entries, not compressed
//	+ 377 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x30 
//	Total size: 512 + 24128 + 1920 = 26560
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TEMPLE_H
#define GRIT_TEMPLE_H

#define templeTilesLen 24128
extern const unsigned int templeTiles[6032];

#define templeMapLen 1920
extern const unsigned short templeMap[960];

#define templePalLen 512
extern const unsigned short templePal[256];

#endif // GRIT_TEMPLE_H

//}}BLOCK(temple)
