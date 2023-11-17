#include "vkbuffer.h"
#include <stdlib.h>
#include <stdio.h>

Buffer createDeviceBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize)
{
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    Buffer buffer = {};
    if (vmaCreateBuffer(
        allocator, 
        &bufferInfo, 
        &allocInfo, 
        &buffer.handle, 
        &buffer.alloc, 
        &buffer.info))
    {
        fprintf(stderr, "Failed to allocate Buffer\n");
        exit(EXIT_FAILURE);
    }

    return buffer;
}