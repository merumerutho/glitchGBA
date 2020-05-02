
//{{BLOCK(bg)

//======================================================================
//
//	bg, 240x160@8, 
//	+ palette 256 entries, not compressed
//	+ 331 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 30x20 
//	Total size: 512 + 21184 + 1200 = 22896
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BG_H
#define GRIT_BG_H

#define bgTilesLen 21184
extern const unsigned int bgTiles[5296];

#define bgMapLen 1200
extern const unsigned short bgMap[600];

#define bgPalLen 512
extern const unsigned short bgPal[256];

#endif // GRIT_BG_H

//}}BLOCK(bg)
