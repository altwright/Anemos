#pragma once
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include "config.h"
#include "model.h"

/*
Vulkan expects the data in your structure to be aligned in memory in a specific way, for example:
    - Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
    - A vec2 must be aligned by 2N (= 8 bytes)
    - A vec3 or vec4 must be aligned by 4N (= 16 bytes)
    - A nested structure must be aligned by the base alignment of its members 
        rounded up to a multiple of 16.
    - A mat4 matrix must have the same alignment as a vec4.
*/
typedef struct UniformBufferData{
    mat4 model;
    mat4 view;
    mat4 projection;
} UniformBufferData;

typedef struct PushConstant{
    mat4 viewProjection;
} PushConstant;

typedef struct View{
    mat4 matrix;
} View;

typedef struct Projection{
    mat4 matrix;
} Projection;

typedef struct DescriptorSets{
    VkDescriptorSetLayout layout;
    VkDescriptorSet handles[MAX_FRAMES_IN_FLIGHT];
} DescriptorSets;

VkShaderModule createShaderModule(VkDevice device, uint32_t *code, size_t numBytes);
VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device);
VkDescriptorPool createDescriptorPool(VkDevice device);
DescriptorSets allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool pool);
void updateUniformBuffer(Buffer *uniformBuffer, VkDeviceSize offset, Model *model);