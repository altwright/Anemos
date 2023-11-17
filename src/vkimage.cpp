#include "vkimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vkstate.h"
#include "vkmemory.h"
#include "vkdevice.h"
#include "int.h"

Image createImage(
    VkDevice device, 
    const VkPhysicalDeviceMemoryProperties *memProperties,
    VkExtent2D extent,
    VkFormat format, 
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags desiredMemProperties,
    VkImageAspectFlags imageAspectFlags,
    VkSampleCountFlagBits sampleCount)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = sampleCount;

    Image image = {};
    image.extent = extent;
    image.format = format;
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

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = imageAspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, NULL, &image.view)){
        fprintf(stderr, "Failed to create Image View\n");
        exit(EXIT_FAILURE);
    }

    return image;
}

Image createDepthBufferImage(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice, 
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount)
{
    size_t candidateFormatsCount = 3;
    VkFormat candidateFormats[candidateFormatsCount] = {
        VK_FORMAT_D32_SFLOAT, 
        VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT};
    VkFormat format = findSupportedFormat(
        physicalDevice->handle, 
        candidateFormats, 
        candidateFormatsCount, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    
    return createImage(
        device, 
        &physicalDevice->memProperties,
        extent,
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        samplingCount);
}