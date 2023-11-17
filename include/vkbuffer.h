#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "vkstate.h"

Buffer createDeviceBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);