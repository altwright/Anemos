#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

void recordDrawCommand(
    VkCommandBuffer commandBuffer, 
    VkRenderPass renderPass, 
    VkFramebuffer framebuffer, 
    VkPipeline graphicsPipeline,
    const SwapchainDetails *swapchainDetails);

void submitDrawCommand(
    VkQueue queue, 
    VkCommandBuffer commandBuffer, 
    VkSemaphore waitSemaphore,
    VkSemaphore signalSemaphore,
    VkFence hostFence);

VkResult presentSwapchain(
    VkQueue presentQueue, 
    VkSemaphore waitSemaphore,
    VkSwapchainKHR swapchain,
    uint32_t swapchainImageIndex);