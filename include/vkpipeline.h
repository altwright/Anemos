#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

VkRenderPass createRenderPass(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice,
    const SwapchainDetails *swapchain,
    const Image *depthImage,
    const Image *samplingImage,
    VkSampleCountFlagBits samplingCount);