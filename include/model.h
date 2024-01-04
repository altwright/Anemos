#pragma once
#include "int.h"
#include "vertex.h"

typedef struct Model{
    u32 verticesCount;
    Vertex *vertices;
    u32 indicesCount;
    u16 *indices;
    mat4 worldMatrix;
} Model;

typedef struct ModelInfo{
    size_t verticesCount;
    size_t verticesDataSize;

    size_t indicesCount;
    size_t indicesDataSize;

    size_t texCoordCount;
    size_t texCoordDataSize;

    int texWidth;
    int texHeight;
    int texChannels;

    mat4 worldMatrix;
} ModelInfo;

Model loadModel(const char *filePath);
void freeModel(Model *model);
ModelInfo loadModelIntoStagingBuffer(const char *glbFilePath, unsigned char *mappedStagingBuffer);