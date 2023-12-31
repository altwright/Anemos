#pragma once
#include <vulkan/vulkan.h>
#include "int.h"
#include "vkstate.h"

VmaAllocator createAllocator(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice);
VkSampler createSampler(VkDevice device, float maxAnisotropy);
Texture createDeviceTexture(
    VkDevice device, 
    VmaAllocator allocator, 
    size_t texWidth, size_t texHeight);
Buffer createDeviceBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);
Buffer createStagingBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);
Buffer createUniformBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);
void copyToDeviceBuffer(
    size_t bytesCount,
    VkBuffer srcBuffer,
    size_t srcOffset,
    VkBuffer dstBuffer,
    size_t dstOffset,
    VkDevice device,
    VkCommandPool transferCmdPool,
    VkQueue transferQueue);
void copyToDeviceTexture(
    VkDevice device,
    VkImage deviceTexture,
    VkBuffer stagingBuffer,
    VkDeviceSize stagingBufferOffset,
    u32 textureWidth,
    u32 textureHeight,
    VkCommandPool transferCmdPool,
    VkQueue transferQueue);