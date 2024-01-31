#pragma once
#include "model.h"

typedef struct{
    size_t vtxBufOffset;
    size_t idxBufOffset;
    size_t drawCmdsOffset;
    size_t drawCmdsCount;

    ModelInfo surfaceModelInfo;
    ModelInfo characterModelInfo;
} SceneInfo;

SceneInfo loadSceneToDevice(
    const char *surfaceFilepath, 
    const char *characterFilepath, 
    Buffer stagingBuffer, Buffer deviceBuffer,
    VkDevice device,
    VmaAllocator allocator,
    VkCommandPool cmdPool,
    VkQueue queue);