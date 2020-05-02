
//{{BLOCK(Immagine)

//======================================================================
//
//	Immagine, 512x264@8, 
//	+ palette 256 entries, not compressed
//	+ 1404 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 64x33 
//	Total size: 512 + 89856 + 4224 = 94592
//
//	Time-stamp: 2020-04-14, 15:29:05
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_IMMAGINE_H
#define GRIT_IMMAGINE_H

#define ImmagineTilesLen 89856
extern const unsigned int ImmagineTiles[22464];

#define ImmagineMapLen 4224
extern const unsigned short ImmagineMap[2112];

#define ImmaginePalLen 512
extern const unsigned short ImmaginePal[256];

#endif // GRIT_IMMAGINE_H

//}}BLOCK(Immagine)
