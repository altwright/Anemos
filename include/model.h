#pragma once
#include "int.h"
#include "vertex.h"
#include "cgltf.h"
#include "vkmemory.h"

typedef struct{
    size_t elementCount;
    size_t dataSize;
} ModelAttributeInfo;

typedef struct{
    u32 width;
    u32 height;
    u32 channels;
} TextureInfo;

typedef struct{
    mat4 worldMatrix;
    DeviceImage tex;
} ModelInfo;

cgltf_data* loadglTFData(const char *glbFilepath);
//ModelInfo stageModelData(const char *glbFilePath, u8 *mappedStagingBuffer);
ModelAttributeInfo stageModelVertexAttributes(cgltf_data *modelData, u8* stagingBuffer);
ModelAttributeInfo stageModelIndices(cgltf_data* modelData, u8* stagingBuffer);
TextureInfo stageModelTexture(cgltf_data *modelData, u8* stagingBuffer);