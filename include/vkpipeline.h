#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

VkRenderPass createRenderPass(
    VkDevice device, 
    VkFormat swapchainFormat,
    VkFormat depthImageFormat,
    VkFormat samplingImageFormat,
    VkSampleCountFlagBits samplingCount);