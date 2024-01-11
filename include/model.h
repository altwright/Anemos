#pragma once
#include "int.h"
#include "vertex.h"

typedef struct ModelInfo{
    size_t verticesCount;
    size_t verticesDataSize;

    size_t indicesCount;
    size_t indicesDataSize;

    size_t texCoordCount;
    size_t texCoordDataSize;

    int texWidth;
    int texHeight;

    mat4 worldMatrix;
} ModelInfo;

ModelInfo loadModelIntoStagingBuffer(const char *glbFilePath, u8 *mappedStagingBuffer);