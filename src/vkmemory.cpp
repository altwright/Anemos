#include "vkmemory.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "vkcommand.h"
#include "config.h"

VmaAllocator createAllocator(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.device = device;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = VULKAN_VERSION;

    VmaAllocator allocator = {};
    if (vmaCreateAllocator(&allocatorInfo, &allocator)){
        fprintf(stderr, "Failed to create Vulkan Memory Allocator");
        exit(EXIT_FAILURE);
    }
    return allocator;
}

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

Buffer createUniformBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize)
{
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;

    Buffer uniformBuffer = {};
    if (vmaCreateBuffer(
        allocator, 
        &bufferInfo, 
        &allocInfo, 
        &uniformBuffer.handle, 
        &uniformBuffer.alloc, 
        &uniformBuffer.info))
    {
        fprintf(stderr, "Failed to allocate Uniform Buffer\n");
        exit(EXIT_FAILURE);
    }

    return uniformBuffer;
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
/*
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
*/

/*
void copyBufferRegion(
    VkDevice device,
    VkCommandPool transientCommandPool,
    VkQueue transferQueue,
    Buffer srcBuffer, 
    Buffer dstBuffer, 
    VkBufferCopy copyRegion)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommand(device, transientCommandPool);
    vkCmdCopyBuffer(commandBuffer, srcBuffer.handle, dstBuffer.handle, 1, &copyRegion);
    submitSingleTimeCommand(device, transientCommandPool, commandBuffer, transferQueue);
}

Buffer createStagingBuffer(VkDevice device, const PhysicalDeviceDetails *physicalDeviceDetails, VkDeviceSize bufferSize){
    return createBuffer(
        device,
        physicalDeviceDetails,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

void copyDataToLocalBuffer(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkCommandPool transferCommandPool,
    VkQueue transferQueue,
    Buffer dstBuffer,
    VkDeviceSize dstBufferOffset,
    const void *dataArray,
    size_t dataCount,
    size_t datatypeSize)
{
    Buffer stagingBuffer = createStagingBuffer(device, physicalDevice, datatypeSize*dataCount);

    void *mappedStagingBuffer = NULL;
    vkMapMemory(device, stagingBuffer.memory, 0, stagingBuffer.size, 0, &mappedStagingBuffer);
    memcpy(mappedStagingBuffer, dataArray, datatypeSize*dataCount);
    vkUnmapMemory(device, stagingBuffer.memory);

    VkBufferCopy bufferCopyRegion = {
        .srcOffset = 0,
        .dstOffset = dstBufferOffset,
        .size = datatypeSize*dataCount
    };

    copyBufferRegion(
        device,
        transferCommandPool, 
        transferQueue,
        stagingBuffer, 
        dstBuffer,
        bufferCopyRegion
    );

    vkDestroyBuffer(device, stagingBuffer.handle, NULL);
    vkFreeMemory(device, stagingBuffer.memory, NULL);
}

void transitionImageLayout(
    VkDevice device, 
    VkCommandPool transientCommandPool,
    VkQueue transferQueue,
    VkImage textureImage,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommand(device, transientCommandPool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } 
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else {
        fprintf(stderr, "Unsupported Image Layout Transition\n");
        exit(EXIT_FAILURE);
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier);

    submitSingleTimeCommand(device, transientCommandPool, commandBuffer, transferQueue);

}

void copyPixelsToLocalImage(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDeviceDetails,
    VkCommandPool transientCommandPool,
    VkQueue transferQueue,
    const void *texelsArray,
    size_t texelSize,
    u32 numTexelRows,
    u32 numTexelCols,
    VkImage dstImage)
{
    Buffer stagingBuffer = createStagingBuffer(
        device, 
        physicalDeviceDetails, 
        texelSize*numTexelRows*numTexelCols);

    void *mappedStagingBuffer = NULL;
    vkMapMemory(device, stagingBuffer.memory, 0, stagingBuffer.size, 0, &mappedStagingBuffer);
    memcpy(mappedStagingBuffer, texelsArray, texelSize*numTexelRows*numTexelCols);
    vkUnmapMemory(device, stagingBuffer.memory);

    VkCommandBuffer commandBuffer = beginSingleTimeCommand(device, transientCommandPool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {numTexelCols, numTexelRows, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer.handle,
        dstImage,
        //I’m assuming here that the image has already been transitioned to the 
        //layout that is optimal for copying pixels to.
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    submitSingleTimeCommand(device, transientCommandPool, commandBuffer, transferQueue);

    vkDestroyBuffer(device, stagingBuffer.handle, NULL);
    vkFreeMemory(device, stagingBuffer.memory, NULL);
}


*/