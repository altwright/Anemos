#include "physics.h"
#include <immintrin.h>
#include "timing.h"

typedef __m256 v256f;

#define GRAVITY_M_S_S 9.82f
vec3 GRAVITY_DIR = {0.0f, -1.0f, 0.0f};

static inline float min(float x, float y) {
    return x < y ? x : y;
}

static inline float max(float x, float y) {
    return x > y ? x : y;
}

static int findIndexOfPointWithinVoxel(vec3 point, const Voxels *voxels)
{
    int colIdx = -1, rowIdx = -1, depthIdx = -1;

    if (point[0] >= voxels->origin[0] && point[0] < (voxels->origin[0] + voxels->voxWidth*voxels->cols))
        colIdx = (point[0] - voxels->origin[0])/(voxels->voxWidth);

    if (point[1] > (voxels->origin[1] - voxels->voxHeight*voxels->rows) && point[1] <= voxels->origin[1])
        rowIdx = (voxels->origin[1] - point[1])/(voxels->voxHeight);

    if (point[2] >= voxels->origin[2] && point[2] < (voxels->origin[2] + voxels->voxLength*voxels->depth))
        depthIdx = (point[2] - voxels->origin[2])/(voxels->voxLength);

    if (colIdx >= 0 && rowIdx >= 0 && depthIdx >= 0)//If it lies within the voxels
        return colIdx*(voxels->rows*voxels->depth) + rowIdx*(voxels->depth) + depthIdx;
    else
        return -1;
}

void updateCharacterPhysics(Character *character, s64 timeDiff_ns)
{
    vec3 fallingVelocity = {};
    glm_vec3_scale(GRAVITY_DIR, GRAVITY_M_S_S*NS_TO_SEC(timeDiff_ns), fallingVelocity);

    glm_vec3_add(character->vel_m_s, fallingVelocity, character->vel_m_s);
}

void applyCharacterSurfaceCollision(Character *character, const Voxels *surface)
{
    float magnitude = glm_vec3_norm(character->vel_m_s);
    vec3 dir = {};
    glm_vec3_scale(character->vel_m_s, 1.0f/magnitude, dir);

    //Detect whether the starting position lies outside the voxel box
    float boxWidth = surface->cols * surface->voxWidth;
    float boxHeight = surface->rows * surface->voxHeight;
    float boxLength = surface->depth * surface->voxLength;
    
    float boxOriginX = surface->origin[0];
    float boxOriginY = surface->origin[1];
    float boxOriginZ = surface->origin[2];

    //Use a ray tracing intersection algorithm
    Box box = {{
        {boxOriginX, boxOriginY, boxOriginZ},
        {boxOriginX + boxWidth, boxOriginY + boxHeight, boxOriginZ + boxLength}
    }};

    Ray ray = {
        {character->pos[0], character->pos[1], character->pos[2]},
        {dir[0], dir[1], dir[2]},
        {1.0f/dir[0], 1.0f/dir[1], 1.0f/dir[2]}
    };

    //ray = origin + t*dir
    //A segment of the ray lies between tmin and tmax.
    //If the test finds that tmin > tmax, then the ray
    //lies outside the box
    float tmin = magnitude;

    rayAABBIntersections(&ray, 1, &box, &tmin);
    
    if (tmin < magnitude)//Ray passes through AABB at point (origin + tmin*dir)
    {
        tmin = magnitude;//Reset for tests within the box

        vec3 projCharacterPos = {};
        glm_vec3_add(character->pos, character->vel_m_s, projCharacterPos);
        int originVoxelIdx = findIndexOfPointWithinVoxel(character->pos, surface);
        int dstVoxelIdx = findIndexOfPointWithinVoxel(projCharacterPos, surface);

        u32 numVoxels = surface->cols * surface->rows * surface->depth;

        u16 *voxelIndexes = (u16*)surface->data;
        u16 *indices = (u16*)(surface->data + sizeof(u16)*numVoxels);
        vec3 *vertices = (vec3*)(surface->data + surface->transformedVerticesIdx);

        if (originVoxelIdx == dstVoxelIdx)//Only need to test one voxel
        {
            u32 maxIndex = originVoxelIdx < numVoxels-1 ? voxelIndexes[originVoxelIdx+1] : surface->storedIndicesCount;

            u16 baseIndex = voxelIndexes[originVoxelIdx];

            for (u32 i = baseIndex; i < maxIndex; i += 3)
            {
                vec3 v1 = {};
                vec3 v2 = {};
                vec3 v3 = {};

                glm_vec3_copy(vertices[indices[i]], v1);
                glm_vec3_copy(vertices[indices[i+1]], v2);
                glm_vec3_copy(vertices[indices[i+2]], v3);

                float intersectionDistance = 0;
                if (glm_ray_triangle(character->pos, dir, v1, v2, v3, &intersectionDistance)
                    && intersectionDistance < tmin)
                {
                    tmin = intersectionDistance;
                }
            }
        }
        else//Test all voxels
        {
            float intersections[4096] = {};
            const v256f vMagnitudes = _mm256_set1_ps(magnitude);
            for (u32 i = 0; i < numVoxels; i += 8)
            {
                _mm256_store_ps(intersections + i, vMagnitudes);
            }

            rayAABBVoxelIntersections(&ray, surface, intersections);

            for (u32 i = 0; i < numVoxels; i++)
            {
                if (intersections[i] < magnitude)
                {
                    u32 maxIndex = i < numVoxels-1 ? voxelIndexes[i+1] : surface->storedIndicesCount;

                    u16 baseIndex = voxelIndexes[i];

                    for (u32 j = baseIndex; j < maxIndex; j += 3)
                    {
                        vec3 v1 = {};
                        vec3 v2 = {};
                        vec3 v3 = {};

                        glm_vec3_copy(vertices[indices[j]], v1);
                        glm_vec3_copy(vertices[indices[j+1]], v2);
                        glm_vec3_copy(vertices[indices[j+2]], v3);

                        float intersectionDistance = 0;
                        if (glm_ray_triangle(character->pos, dir, v1, v2, v3, &intersectionDistance)
                            && intersectionDistance < tmin)
                        {
                            tmin = intersectionDistance;
                        }
                    }
                }
            }
        }

        if (tmin < magnitude)//Collided
        {
            glm_vec3_scale(dir, tmin, dir);
            glm_vec3_add(character->pos, dir, character->pos);
            glm_vec3_zero(character->vel_m_s);
        }
        else
        {
            glm_vec3_add(character->pos, character->vel_m_s, character->pos);
        }
    }
    else
    {
        glm_vec3_add(character->pos, character->vel_m_s, character->pos);
    }
}

//Only designed to detect the point where a ray comes in contact with a box
//https://tavianator.com/2022/ray_box_boundary.html
void rayAABBIntersections(const Ray *ray, size_t nboxes, const Box boxes[], float ts[])
{
    bool signs[3];
    for (int d = 0; d < 3; ++d) {
        signs[d] = signbit(ray->dirRcp[d]);
    }

    for (size_t i = 0; i < nboxes; ++i) {
        const Box *box = &boxes[i];
        float tmin = 0.0, tmax = ts[i];

        for (int d = 0; d < 3; ++d) {
            float bmin = box->corners[signs[d]][d];
            float bmax = box->corners[!signs[d]][d];

            float dmin = (bmin - ray->origin[d]) * ray->dirRcp[d];
            float dmax = (bmax - ray->origin[d]) * ray->dirRcp[d];

            tmin = max(dmin, tmin);
            tmax = min(dmax, tmax);
        }

        ts[i] = tmin <= tmax ? tmin : ts[i];
    }
}

void rayAABBVoxelIntersections(const Ray *ray, const Voxels *voxels, float ts[])
{
    bool signs[3];
    for (u32 d = 0; d < 3; ++d) {
        signs[d] = signbit(ray->dirRcp[d]);
    }

    for (u32 col = 0; col < voxels->cols; col++)
    {
        for (u32 row = 0; row < voxels->rows; row++)
        {
            for (u32 depth = 0; depth < voxels->depth; depth++)
            {
                const Box box = {{
                    {
                        voxels->origin[0] + col*voxels->voxWidth, 
                        voxels->origin[1] + row*voxels->voxHeight, 
                        voxels->origin[2] + depth*voxels->voxLength
                    }, 
                    {
                        voxels->origin[0] + (col+1)*voxels->voxWidth, 
                        voxels->origin[1] + (row+1)*voxels->voxHeight, 
                        voxels->origin[2] + (depth+1)*voxels->voxLength
                    }
                }};

                u32 voxIdx = col*(voxels->rows*voxels->depth) + row*(voxels->depth) + depth;
                float tmin = 0.0, tmax = ts[voxIdx];

                for (int d = 0; d < 3; ++d) {
                    float bmin = box.corners[signs[d]][d];
                    float bmax = box.corners[!signs[d]][d];

                    float dmin = (bmin - ray->origin[d]) * ray->dirRcp[d];
                    float dmax = (bmax - ray->origin[d]) * ray->dirRcp[d];

                    tmin = max(dmin, tmin);
                    tmax = min(dmax, tmax);
                }

                ts[voxIdx] = tmin <= tmax ? tmin : ts[voxIdx];
            }
        }
    }
}