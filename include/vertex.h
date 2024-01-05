#pragma once
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include "int.h"
#include "vkstate.h"

#define VERTEX_ATTRIBUTE_COUNT 2

typedef struct VertexAttributes{
    vec3 position;
    vec2 texCoord;
} VertexAttributes;

typedef struct VertexInputAttributeDescriptions{
    VkVertexInputAttributeDescription descs[VERTEX_ATTRIBUTE_COUNT];
} VertexInputAttributeDescriptions;

VkVertexInputBindingDescription getVertexBindingDescription();
VertexInputAttributeDescriptions getVertexInputAttributes();