#pragma once
#include <vulkan/vulkan.h>

VkInstance createInstance(const char *appName, uint32_t appVersion, const char *engineName, uint32_t engineVersion);
VkPhysicalDevice selectPhysicalDevice(VkInstance instance);
struct QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device);
VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice);