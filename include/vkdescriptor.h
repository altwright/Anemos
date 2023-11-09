#pragma once
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

typedef struct UniformBufferData{
    mat4 model;
    mat4 view;
    mat4 projection;
} UniformBufferData;

void updateUniformBuffer(void *mappedUniformBuffer, VkExtent2D swapchainExtent);