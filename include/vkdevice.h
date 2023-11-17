#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

PhysicalDeviceDetails selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
VkDevice createLogicalDevice(const PhysicalDeviceDetails *physicalDevice);
VkFormat findSupportedFormat(
    VkPhysicalDevice device, 
    VkFormat *candidateFormats, 
    size_t candidateFormatsCount,
    VkImageTiling tiling,
    VkFormatFeatureFlags desiredFeatures);