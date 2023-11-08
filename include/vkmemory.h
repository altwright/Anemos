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