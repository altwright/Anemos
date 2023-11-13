#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

Image createImage(
    VkDevice device, 
    const VkPhysicalDeviceMemoryProperties *memProperties,
    VkExtent2D extent,
    VkFormat format, 
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags desiredMemProperties,
    VkImageAspectFlags imageAspectFlags);