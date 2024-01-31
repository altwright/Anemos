#include "vkattachment.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vkstate.h"
#include "vkmemory.h"
#include "vkdevice.h"
#include "int.h"

DeviceImage createDepthImage(
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

    DeviceImage depthImage = {};
    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &depthImage.handle, &depthImage.alloc, &depthImage.info)){
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

DeviceImage createSamplingImage(
    VmaAllocator allocator, 
    VkDevice device, 
    VkFormat format, 
    VkExtent2D extent,
    VkSampleCountFlagBits samplingCount)
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
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = samplingCount;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    DeviceImage samplingImage = {};
    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &samplingImage.handle, &samplingImage.alloc, &samplingImage.info)){
        fprintf(stderr, "Failed to allocate Image\n");
        exit(EXIT_FAILURE);
    }
    
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = samplingImage.handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, NULL, &samplingImage.view)){
        fprintf(stderr, "Failed to create Image View\n");
        exit(EXIT_FAILURE);
    }

    samplingImage.format = format;
    samplingImage.extent = extent;

    return samplingImage;
}

Framebuffers createFramebuffers(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    VkImageView depthImageView,
    VkImageView samplingImageView)
{
    Framebuffers framebuffers{};
    framebuffers.count = swapchainDetails->imagesCount;
    framebuffers.handles = (VkFramebuffer*)malloc(sizeof(VkFramebuffer)*framebuffers.count);

    for(size_t i = 0; i < framebuffers.count; i++){
        //The color attachment differs for every swap chain image, but the same depth image 
        //can be used by all of them because only a single subpass is running at the same 
        //time due to our semaphores.
        u32 attachmentsCount = 3;
        VkImageView attachments[attachmentsCount] = {
            samplingImageView, 
            depthImageView, 
            swapchainDetails->imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        //You can only use a framebuffer with the render passes that it is compatible 
        //with, which roughly means that they use the same number and type of attachments.
        framebufferInfo.attachmentCount = attachmentsCount;
        framebufferInfo.pAttachments = attachments;
        //The attachmentCount and pAttachments parameters specify the VkImageView objects 
        //that should be bound to the respective attachment descriptions in the render pass pAttachment array.
        framebufferInfo.width = swapchainDetails->extent.width;
        framebufferInfo.height = swapchainDetails->extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &framebuffers.handles[i])){
            printf("Failed to create Framebuffer %ld\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return framebuffers;
}

void destroyFramebuffers(VkDevice device, Framebuffers *framebuffers)
{
    for(size_t i = 0; i < framebuffers->count; i++)
        vkDestroyFramebuffer(device, framebuffers->handles[i], NULL);
    free(framebuffers->handles);
}