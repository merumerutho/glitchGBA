
//{{BLOCK(palace)

//======================================================================
//
//	palace, 256x232@8, 
//	+ palette 256 entries, not compressed
//	+ 482 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x29 
//	Total size: 512 + 30848 + 1856 = 33216
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_PALACE_H
#define GRIT_PALACE_H

#define palaceTilesLen 30848
extern const unsigned int palaceTiles[7712];

#define palaceMapLen 1856
extern const unsigned short palaceMap[928];

#define palacePalLen 512
extern const unsigned short palacePal[256];

#endif // GRIT_PALACE_H

//}}BLOCK(palace)
