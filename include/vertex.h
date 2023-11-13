#pragma once
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include "int.h"
#include "vkstate.h"

#define VERTEX_ATTRIBUTE_COUNT 3

typedef struct Vertex{
    vec3 pos;
    vec3 colour;
    vec2 texCoord;
} Vertex;

typedef struct VertexInputAttributes{
    VkVertexInputAttributeDescription descriptions[VERTEX_ATTRIBUTE_COUNT];
} VertexInputAttributes;

#define VERTEX_COUNT 8
extern const Vertex vertices[VERTEX_COUNT];
#define INDEX_COUNT 12
extern const u16 indices[INDEX_COUNT];

VkVertexInputBindingDescription getVertexBindingDescription();
VertexInputAttributes getVertexInputAttributes();
void copyVerticesToStagingBuffer(VkDevice device, Buffer vertexBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount);
void copyIndicesToStagingBuffer(VkDevice device, Buffer indexBuffer, VkDeviceSize offset, const u16 *indices, size_t indicesCount);