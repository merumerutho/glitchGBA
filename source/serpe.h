
//{{BLOCK(serpe)

//======================================================================
//
//	serpe, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 255 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x32 
//	Total size: 512 + 16320 + 2048 = 18880
//
//	Time-stamp: 2020-04-28, 17:17:38
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_SERPE_H
#define GRIT_SERPE_H

#define serpeTilesLen 16320
extern const unsigned int serpeTiles[4080];

#define serpeMapLen 2048
extern const unsigned short serpeMap[1024];

#define serpePalLen 512
extern const unsigned short serpePal[256];

#endif // GRIT_SERPE_H

//}}BLOCK(serpe)
