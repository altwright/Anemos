#include "vkshader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timing.h"
#include "int.h"
#include "vkstate.h"

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding ubBinding = {};
    ubBinding.binding = 0;
    ubBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubBinding.descriptorCount = 1;//Specifies num elements in array
    ubBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 2;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {ubBinding, samplerBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = NUM_ELEMENTS(bindings);
    layoutInfo.pBindings = bindings;

    VkDescriptorSetLayout layout = {};
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &layout)){
        fprintf(stderr, "Failed to create Descriptor Set Layout\n");
        exit(EXIT_FAILURE);
    }

    return layout;
}

VkDescriptorPool createDescriptorPool(VkDevice device)
{
    VkDescriptorPoolSize ubPoolSize = {};
    ubPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubPoolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;//Max descriptors, distributed between the sets

    VkDescriptorPoolSize samplerPoolSize = {};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolSize poolSizes[] = {ubPoolSize, samplerPoolSize};

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = NUM_ELEMENTS(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPool descriptorPool = NULL;
    if (vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool)){
        fprintf(stderr, "Failed to create Descriptor Pool\n");
        exit(EXIT_FAILURE);
    }

    return descriptorPool;
}

DescriptorSets allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool pool)
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {layout, layout};

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    DescriptorSets sets = {};
    sets.layout = layout;
    if (vkAllocateDescriptorSets(device, &allocInfo, sets.handles)){
        fprintf(stderr, "Failed to allocate Descriptor Sets\n");
        exit(EXIT_FAILURE);
    }

    return sets;
}

void updateUniformBuffer(Buffer *uniformBuffer, VkDeviceSize offset, ModelInfo *modelInfo)
{
    u8 *mappedBuffer = (u8*)uniformBuffer->info.pMappedData + offset;
    memcpy(mappedBuffer, modelInfo->modelMatrix, sizeof(mat4));
}

VkShaderModule createShaderModule(VkDevice device, uint32_t *code, size_t numBytes)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = numBytes;
    createInfo.pCode = code;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, NULL, &module)){
        printf("Failed to create Shader Module of byte size %ld\n", numBytes);
        exit(EXIT_FAILURE);
    }

    return module;
}