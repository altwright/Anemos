#pragma once
#include <vulkan/vulkan.h>
#include "int.h"

//UINT32_MAX means memory type was not found
u32 findVkMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties *memProperties, VkMemoryPropertyFlags desiredPropertyFlags);