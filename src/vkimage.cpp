#include "vkimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vkstate.h"
#include "vkmemory.h"
#include "vkdevice.h"
#include "int.h"

Image createDepthImage(
    VmaAllocator allocator,
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount)
{
    size_t candidateFormatsCount = 3;
    VkFormat candidateFormats[candidateFormatsCount] = {
        VK_FORMAT_D32_SFLOAT, 
        VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT};
    VkFormat format = findSupportedFormat(
        physicalDevice, 
        candidateFormats, 
        candidateFormatsCount, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (format == VK_FORMAT_MAX_ENUM){
        fprintf(stderr, "Failed to find Depth Buffer format\n");
        exit(EXIT_FAILURE);
    }

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = samplingCount;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    Image depthImage = {};
    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthImage.handle, &depthImage.alloc, NULL)){
        fprintf(stderr, "Failed to allocate Image\n");
        exit(EXIT_FAILURE);
    }
    
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImage.handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, NULL, &depthImage.view)){
        fprintf(stderr, "Failed to create Image View\n");
        exit(EXIT_FAILURE);
    }

    depthImage.format = format;
    depthImage.extent = extent;

    return depthImage;
}