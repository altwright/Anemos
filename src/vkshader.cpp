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

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &ubBinding;

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
    ubPoolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;//Max descriptors, shared between the sets

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &ubPoolSize;
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

void updateUniformBuffer(Buffer *uniformBuffer, VkDeviceSize offset, Model *model)
{
    unsigned char *mappedBuffer = (unsigned char*)uniformBuffer->info.pMappedData + offset;
    memcpy(mappedBuffer, model->worldMatrix, sizeof(mat4));
}

void updateUniformBuffer(void *mappedUniformBuffer, VkExtent2D swapchainExtent)
{
    static const s64 startTimeNs = SEC_TO_NS(START_TIME.tv_sec) + START_TIME.tv_nsec;

    timespec currentTime = {};
    if (clock_gettime(TIMING_CLOCK, &currentTime)){
        perror("Failed to get Current Time\n");
        exit(EXIT_FAILURE);
    }

    s64 currentTimeNs = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiffNs = currentTimeNs - startTimeNs;
    float rotationRadians = (glm_rad(90.0f) * timeDiffNs)/SEC_TO_NS(1);

    UniformBufferData ubo = {};
    glm_mat4_identity(ubo.model);
    vec3 rotationAxis = {0.0f, 0.0f, 1.0f};
    glm_rotate(ubo.model, rotationRadians, rotationAxis);
    vec3 eye = {2.0f, 2.0f, 2.0f};
    vec3 centre = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_lookat(eye, centre, up, ubo.view);
    glm_perspective(glm_rad(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f, ubo.projection);
    ubo.projection[1][1] *= -1;//Originally designed for OpenGL, so must be inverted

    memcpy(mappedUniformBuffer, &ubo, sizeof(UniformBufferData));
    //Push Constants are a more efficient way of transferring small data to the shaders
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