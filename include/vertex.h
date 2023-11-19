#pragma once
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include "int.h"
#include "vkstate.h"

#define VERTEX_ATTRIBUTE_COUNT 1

typedef struct Vertex{
    vec3 position;
} Vertex;

typedef struct VertexInputAttributes{
    VkVertexInputAttributeDescription descriptions[VERTEX_ATTRIBUTE_COUNT];
} VertexInputAttributes;

VkVertexInputBindingDescription getVertexBindingDescription();
VertexInputAttributes getVertexInputAttributes();
void copyVerticesToStagingBuffer(VkDevice device, Buffer vertexBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount);
void copyIndicesToStagingBuffer(VkDevice device, Buffer indexBuffer, VkDeviceSize offset, const u16 *indices, size_t indicesCount);