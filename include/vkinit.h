#pragma once
#include <vulkan/vulkan.h>

VkInstance createInstance(const char *appName, uint32_t appVersion, const char *engineName, uint32_t engineVersion);
VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
struct QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices);
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);