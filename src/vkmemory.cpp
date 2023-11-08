#include <vulkan/vulkan.h>
#include "int.h"

u32 findVkMemoryType(u32 typeFilter, const VkPhysicalDeviceMemoryProperties *memProperties, VkMemoryPropertyFlags desiredPropertyFlags)
{
    for (u32 i = 0; i < memProperties->memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && 
            (memProperties->memoryTypes[i].propertyFlags & desiredPropertyFlags) == desiredPropertyFlags)
            return i;
    }

    return UINT32_MAX;
}