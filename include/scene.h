#pragma once
#include "model.h"
#include "physics.h"

typedef struct{
    size_t vtxBufOffset;
    size_t idxBufOffset;
    size_t drawCmdsOffset;
    size_t drawCmdsCount;

    ModelInfo surfaceModelInfo;
    ModelInfo characterModelInfo;

    Voxels surfaceVoxels;
} SceneInfo;

SceneInfo loadSceneToDevice(
    const char *surfaceFilepath, 
    const char *characterFilepath, 
    Buffer stagingBuffer, Buffer deviceBuffer,
    VkDevice device,
    VmaAllocator allocator,
    VkCommandPool cmdPool,
    VkQueue queue);

void freeSceneInfo(SceneInfo *info);
Voxels calcSurfaceVoxels(cgltf_data *surfaceData, mat4 modelMatrix);