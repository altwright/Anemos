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
VkRenderPass createRenderPass(
    VkDevice device, 
    const SwapchainDetails *swapchainDetails,
    const PhysicalDeviceDetails *physicalDevice,
    VkFormat depthBufferFormat);
VkDescriptorPool createDescriptorPool(VkDevice device, u32 numFramesInFlight);
Descriptors createDescriptors(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkDescriptorPool descriptorPool,
    VkSampler textureSampler,
    Image texture,
    size_t numFramesInFlight);
PipelineDetails createGraphicsPipeline(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    Descriptors descriptors,
    VkSampleCountFlagBits sampleCount);
VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags createFlags);
VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool commandPool);
Synchronisers createSynchronisers(VkDevice device);
FrameControllers* createFrameStates(VkDevice device, VkCommandPool commandPool, size_t numFrames);
Framebuffers createFramebuffers(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    Image depthBuffer,
    Image sampleImage);
void recreateSwapchain(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkSurfaceKHR surface,
    VkRenderPass renderPass,
    GLFWwindow *window,
    SwapchainDetails *swapchainDetails,
    Framebuffers *framebuffers,
    Image *depthBuffer,
    Image *sampleImage);
Image createTexture(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDeviceDetails,
    VkCommandPool transientCommandPool,
    VkQueue transferQueue);
VkSampler createTextureSampler(VkDevice device, const PhysicalDeviceDetails *physicalDevice);
Image createDepthBuffer(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice, 
    VkExtent2D swapchainExtent,
    VkSampleCountFlagBits sampleCount);
Image createSampleImage(VkDevice device, const PhysicalDeviceDetails *physicalDevice, SwapchainDetails swapchain);
VkState _initVkState(const Window *window, size_t verticesCount, size_t indicesCount);