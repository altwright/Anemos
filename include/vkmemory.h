#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "int.h"

typedef struct {
    VkBuffer handle;
    VmaAllocation alloc;
    VmaAllocationInfo info;
} Buffer;

typedef struct {
    VkImage handle;
    VmaAllocation alloc;
    VmaAllocationInfo info;
    VkImageView view;
    VkExtent2D extent;
    VkFormat format;
} DeviceImage;

typedef struct MemoryTransferEssentials {
    VkDevice device;
    VkCommandPool cmdPool;
    VkQueue queue;
} MemoryTransferEssentials;

VmaAllocator createAllocator(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice);
VkSampler createSampler(VkDevice device, float maxAnisotropy);
DeviceImage createDeviceTexture(
    VkDevice device, 
    VmaAllocator allocator, 
    u32 texWidth, u32 texHeight);
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