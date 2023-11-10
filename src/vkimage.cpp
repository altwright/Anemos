#include "vkimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vkstate.h"
#include "vkmemory.h"
#include "int.h"

Image createImage(
    VkDevice device, 
    const VkPhysicalDeviceMemoryProperties *memProperties,
    u32 width,
    u32 height,
    u32 channels,
    VkFormat format, 
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags desiredMemProperties)
{
    assert(channels == 4);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    Image image = {};
    image.width = width;
    image.height = height;
    image.channels = channels;
    if (vkCreateImage(device, &imageInfo, NULL, &image.handle)){
        fprintf(stderr, "Failed to create Texture Image\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements(device, image.handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findVkMemoryType(
        memRequirements.memoryTypeBits, 
        memProperties, 
        desiredMemProperties
    );
    
    if (vkAllocateMemory(device, &allocInfo, NULL, &image.memory)){
        fprintf(stderr, "Failed to allocate Texture memory\n");
        exit(EXIT_FAILURE);
    }

    vkBindImageMemory(device, image.handle, image.memory, 0);

    return image;
}