#include "graphics.h"

const unsigned int TilesLen[] = {
   coastTilesLen,
   dbzTilesLen,
   ffta1TilesLen,
   ffta2TilesLen,
   palaceTilesLen,
   pikachuTilesLen,
   pyramidTilesLen,
   serpeTilesLen,
   templeTilesLen,
};

const unsigned short MapsLen[] = {
   coastMapLen,
   dbzMapLen,
   ffta1MapLen,
   ffta2MapLen,
   palaceMapLen,
   pikachuMapLen,
   pyramidMapLen,
   serpeMapLen,
   templeMapLen,
};

const unsigned short PalsLen[] = {
   coastPalLen,
   dbzPalLen,
   ffta1PalLen,
   ffta2PalLen,
   palacePalLen,
   pikachuPalLen,
   pyramidPalLen,
   serpePalLen,
   templePalLen,
};

const unsigned int * const tilesPointer[] __attribute__((aligned(4))) = {
    coastTiles,
    dbzTiles,
    ffta1Tiles,
    ffta2Tiles,
    palaceTiles,
    pikachuTiles,
    pyramidTiles,
    serpeTiles,
    templeTiles,
};

const unsigned short * const mapsPointer[] __attribute__((aligned(4))) = {
    coastMap,
    dbzMap,
    ffta1Map,
    ffta2Map,
    palaceMap,
    pikachuMap,
    pyramidMap,
    serpeMap,
    templeMap,
};

const unsigned short * const palsPointer[] __attribute__((aligned(4))) = {
    coastPal,
    dbzPal,
    ffta1Pal,
    ffta2Pal,
    palacePal,
    pikachuPal,
    pyramidPal,
    serpePal,
    templePal,
};

