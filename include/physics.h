#pragma once
#include "int.h"
#include "cglm/cglm.h"
#include "entities.h"

typedef struct{
    u8* data;
    size_t dataSize;
    size_t transformedVerticesIdx;
    
    u32 storedIndicesCount;

    u32 cols;
    u32 rows;
    u32 depth;

    vec3 origin;//Represents top left corner position of volume
    float voxWidth;
    float voxHeight;
    float voxLength;
} Voxels;

typedef struct {
    vec3 origin;
    vec3 dir;
    vec3 dirRcp;//Reciprocal: dirRcp[d] = 1/dir[d]
} Ray;

typedef struct {
    vec3 corners[2];
} Box;

void updateCharacterPhysics(Character *character, s64 timeDiff_ns);
void applyCharacterSurfaceCollision(Character *character, const Voxels *surface);
void rayAABBIntersections(const Ray *ray, size_t nboxes, const Box boxes[], float ts[]);
void rayAABBVoxelIntersections(const Ray *ray, const Voxels *voxels, float ts[]);