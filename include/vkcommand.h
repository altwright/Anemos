#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"
#include "int.h"

typedef struct CommandBufferResources{
    VkDevice device;
    VkCommandPool commandPool;
    VkQueue queue;
} CommandBufferResources;

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags createFlags);
void recordDrawCommand(
    VkCommandBuffer commandBuffer, 
    VkRenderPass renderPass, 
    VkFramebuffer framebuffer, 
    PipelineDetails graphicsPipeline,
    const SwapchainDetails *swapchainDetails,
    VkDescriptorSet descriptorSet,
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
VkCommandBuffer beginSingleTimeCommand(VkDevice device, VkCommandPool transientCommandPool);
void submitSingleTimeCommand(
    VkDevice device, 
    VkCommandPool transientCommandPool, 
    VkCommandBuffer commandBuffer, 
    VkQueue queue);