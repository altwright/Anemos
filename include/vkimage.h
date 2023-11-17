#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

Image createDepthImage(
    VmaAllocator allocator,
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount);
Image createSamplingImage(
    VmaAllocator allocator, 
    VkDevice device, 
    VkFormat format, 
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount);