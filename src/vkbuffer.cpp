#include "vkbuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include "vkcommand.h"

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

    Buffer deviceBuffer = {};
    if (vmaCreateBuffer(
        allocator, 
        &bufferInfo, 
        &allocInfo, 
        &deviceBuffer.handle, 
        &deviceBuffer.alloc, 
        &deviceBuffer.info))
    {
        fprintf(stderr, "Failed to allocate Device Buffer\n");
        exit(EXIT_FAILURE);
    }

    return deviceBuffer;
}

Buffer createStagingBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize)
{
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;

    Buffer stagingBuffer = {};
    if (vmaCreateBuffer(
        allocator, 
        &bufferInfo, 
        &allocInfo, 
        &stagingBuffer.handle, 
        &stagingBuffer.alloc, 
        &stagingBuffer.info))
    {
        fprintf(stderr, "Failed to allocate Staging Buffer\n");
        exit(EXIT_FAILURE);
    }

    return stagingBuffer;
}

void copyToDeviceBuffer(
    size_t bytesCount,
    VkBuffer srcBuffer,
    size_t srcOffset,
    VkBuffer dstBuffer,
    size_t dstOffset,
    VkDevice device,
    VkCommandPool transferCmdPool,
    VkQueue transferQueue)
{
    VkBufferCopy copyRegion = {
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = bytesCount
    };

    VkCommandBuffer cmdBuffer = beginSingleTimeCommandBuffer(device, transferCmdPool);
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    submitCommandBuffer(device, transferCmdPool, cmdBuffer, transferQueue);
}