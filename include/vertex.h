#pragma once
#include <cglm/call.h>
#include <vulkan/vulkan.h>
#include "int.h"
#include "vkstate.h"

typedef struct Vertex{
    vec2 pos;
    vec3 colour;
} Vertex;

typedef struct VertexInputAttributes{
    u32 count;
    VkVertexInputAttributeDescription *descriptions;//free
} VertexInputAttributes;

#define VERTEX_COUNT 4
extern const Vertex vertices[VERTEX_COUNT];
#define INDEX_COUNT 6
extern const u16 indices[INDEX_COUNT];

VkVertexInputBindingDescription getVertexBindingDescription();
VertexInputAttributes getVertexInputAttributes();
void copyVerticesToStagingBuffer(VkDevice device, Buffer vertexBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount);
void copyIndicesToStagingBuffer(VkDevice device, Buffer indexBuffer, VkDeviceSize offset, const u16 *indices, size_t indicesCount);