#include "vkmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

Buffer createBuffer(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkDeviceSize bufferSize, 
    VkBufferUsageFlags bufferUsage,
    VkMemoryPropertyFlags desiredProperties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Buffer buffer = {};
    if (vkCreateBuffer(device, &bufferInfo, NULL, &buffer.handle)){
        fprintf(stderr, "Failed to allocate Buffer\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements(device, buffer.handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findVkMemoryType(
        memRequirements.memoryTypeBits, 
        &physicalDevice->memProperties, 
        desiredProperties);

    if (allocInfo.memoryTypeIndex == UINT32_MAX){
        fprintf(stderr, "Failed to find suitable memory type for Buffer\n");
        exit(EXIT_FAILURE);
    }

    if (vkAllocateMemory(device, &allocInfo, NULL, &buffer.memory)){
        fprintf(stderr, "Failed to allocate memory for Buffer\n");
        exit(EXIT_FAILURE);
    }

    if (vkBindBufferMemory(device, buffer.handle, buffer.memory, 0)){
        fprintf(stderr, "Failed to bind memory to Buffer\n");
        exit(EXIT_FAILURE);
    }

    buffer.size = bufferInfo.size;
    buffer.physicalSize = memRequirements.size;
    return buffer;
}

void copyBufferRegion(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue transferQueue,
    Buffer srcBuffer, 
    Buffer dstBuffer, 
    VkBufferCopy copyRegion)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = NULL;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)){
        fprintf(stderr, "Failed to allocate Buffer Region Copy Command Buffer\n");
        exit(EXIT_FAILURE);
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdCopyBuffer(commandBuffer, srcBuffer.handle, dstBuffer.handle, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE)){
        fprintf(stderr, "Failed to submit region copy Command Buffer\n");
        exit(EXIT_FAILURE);
    }

    if (vkQueueWaitIdle(transferQueue)){
        fprintf(stderr, "Waiting for region copy command failed\n");
        exit(EXIT_FAILURE);
    }

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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