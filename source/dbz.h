
//{{BLOCK(dbz)

//======================================================================
//
//	dbz, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 442 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x32 
//	Total size: 512 + 28288 + 2048 = 30848
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_DBZ_H
#define GRIT_DBZ_H

#define dbzTilesLen 28288
extern const unsigned int dbzTiles[7072];

#define dbzMapLen 2048
extern const unsigned short dbzMap[1024];

#define dbzPalLen 512
extern const unsigned short dbzPal[256];

#endif // GRIT_DBZ_H

//}}BLOCK(dbz)
