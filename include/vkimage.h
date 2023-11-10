#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

Image createImage(
    VkDevice device, 
    const VkPhysicalDeviceMemoryProperties *memProperties,
    u32 width,
    u32 height,
    u32 channels,
    VkFormat format, 
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags desiredMemProperties);