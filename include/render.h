#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"
#include "int.h"

void recordDrawCommand(
    VkCommandBuffer commandBuffer, 
    VkRenderPass renderPass, 
    VkFramebuffer framebuffer, 
    VkPipeline graphicsPipeline,
    const SwapchainDetails *swapchainDetails,
    Buffer vertexBuffer,
    VkDeviceSize vertexBufferOffset,
    u32 vertexCount,
    Buffer indexBuffer,
    VkDeviceSize indexBufferOffset,
    u32 indexCount);

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