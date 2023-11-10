#pragma once
#include <vulkan/vulkan.h>
#include "int.h"
#include "vkstate.h"

//UINT32_MAX means memory type was not found
u32 findVkMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties *memProperties, VkMemoryPropertyFlags desiredPropertyFlags);
Buffer createBuffer(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkDeviceSize bufferSize, 
    VkBufferUsageFlags bufferUsage,
    VkMemoryPropertyFlags desiredProperties);
void copyBufferRegion(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue transferQueue,
    Buffer srcBuffer, 
    Buffer dstBuffer, 
    VkBufferCopy copyRegion);
Buffer createStagingBuffer(VkDevice device, PhysicalDeviceDetails *physicalDeviceDetails, VkDeviceSize bufferSize);
void copyDataToLocalBuffer(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkCommandPool transferCommandPool,
    VkQueue transferQueue,
    Buffer dstBuffer,
    VkDeviceSize dstBufferOffset,
    const void *dataArray,
    size_t dataCount,
    size_t datatypeSize);
void transitionImageLayout(
    VkDevice device, 
    VkCommandPool transientCommandPool,
    VkQueue transferQueue,
    VkImage texture,
    VkImageLayout oldLayout,
    VkImageLayout newLayout);
void copyPixelsToLocalImage(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDeviceDetails,
    VkCommandPool transientCommandPool,
    VkQueue transferQueue,
    const void *texelsArray,
    size_t texelSize,
    u32 numTexelRows,
    u32 numTexelCols,
    VkImage dstImage);
