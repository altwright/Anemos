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

#define VERTEX_COUNT 3
extern const Vertex vertices[VERTEX_COUNT];

VkVertexInputBindingDescription getVertexBindingDescription();
VertexInputAttributes getVertexInputAttributes();
void copyVerticesToCoherentBuffer(VkDevice device, Buffer vertexBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount);