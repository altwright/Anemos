#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"
#include "window.h"
#include "int.h"

VkInstance createInstance(const char *appName, uint32_t appVersion, const char *engineName, uint32_t engineVersion);
PhysicalDeviceDetails selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices);
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);
SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapchainDetails createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window);
VkRenderPass createRenderPass(VkDevice device, const SwapchainDetails *swapchainDetails);
VkDescriptorPool createDescriptorPool(VkDevice device, u32 numFramesInFlight);
Descriptors createDescriptors(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkDescriptorPool descriptorPool,
    size_t numFramesInFlight);
PipelineDetails createGraphicsPipeline(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    Descriptors descriptors);
Framebuffers createFramebuffers(VkDevice device, VkRenderPass renderPass, const SwapchainDetails *swapchainDetails);
VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags createFlags);
VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool commandPool);
Synchronisers createSynchronisers(VkDevice device);
FrameState* createFrameStates(VkDevice device, VkCommandPool commandPool, size_t numFrames);
void recreateSwapchain(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkRenderPass renderPass,
    GLFWwindow *window,
    SwapchainDetails *swapchainDetails,
    Framebuffers *framebuffers);
VkState initVkState(const Window *window);