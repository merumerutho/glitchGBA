#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "coast.h"
#include "dbz.h"
#include "ffta1.h"
#include "ffta2.h"
#include "palace.h"
#include "pikachu.h"
#include "pyramid.h"
#include "serpe.h"
#include "temple.h"

#define NTILES 9

// Array length declarations
extern const unsigned int TilesLen[];
extern const unsigned short MapsLen[];
extern const unsigned short PalsLen[];

// Pointer array declarations
extern const unsigned int * const tilesPointer[] __attribute__((aligned(4)));
extern const unsigned short * const mapsPointer[] __attribute__((aligned(4)));
extern const unsigned short * const palsPointer[] __attribute__((aligned(4)));

#endif

