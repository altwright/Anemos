#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

VkInstance createInstance(const char *appName, uint32_t appVersion, const char *engineName, uint32_t engineVersion);
VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);
SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapchainDetails createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window);
void createGraphicsPipeline(VkDevice device);