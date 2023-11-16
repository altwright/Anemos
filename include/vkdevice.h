#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"

PhysicalDeviceDetails selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);