#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

DeviceImage createDepthImage(
    VmaAllocator allocator,
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount);
DeviceImage createSamplingImage(
    VmaAllocator allocator, 
    VkDevice device, 
    VkFormat format, 
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount);
Framebuffers createFramebuffers(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    VkImageView depthBufferView,
    VkImageView samplingImageView);
void destroyFramebuffers(VkDevice device, Framebuffers *framebuffers);