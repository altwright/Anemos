#include "vkswapchain.h"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window)
{
    VkSurfaceKHR surface = {};
    if (glfwCreateWindowSurface(instance, window, NULL, &surface)){
        fprintf(stderr, "Failed to create Surface\n");
        exit(EXIT_FAILURE);
    }

    return surface;
}

SwapchainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details{};
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities)){
        printf("Failed to get Surface capabilities\n");
        exit(EXIT_FAILURE);
    }

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.formatsCount, NULL);
    details.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR)*details.formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.formatsCount, details.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModesCount, NULL);
    details.presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR)*details.presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModesCount, details.presentModes);

    return details;
}

void destroySwapchainSupportDetails(SwapchainSupportDetails *details)
{
    if (details->formats)
        free(details->formats);
    if (details->presentModes)
        free(details->presentModes);
}

VkSurfaceFormatKHR selectSurfaceFormat(uint32_t formatsCount, VkSurfaceFormatKHR *availableFormats){
    for (size_t i = 0; i < formatsCount; i++){
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormats[i];
    }
    
    return availableFormats[0];
}

VkPresentModeKHR selectPresentMode(uint32_t presentModesCount, VkPresentModeKHR *availablePresentModes){
    for (size_t i = 0; i < presentModesCount; i++){
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentModes[i];
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D selectSwapchainExtent(GLFWwindow *window, VkSurfaceCapabilitiesKHR *capabilities){
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;
    else{
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

SwapchainDetails createSwapchain(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice, 
    VkSurfaceKHR surface, 
    GLFWwindow *window)
{
    SwapchainSupportDetails supportDetails = querySwapchainSupportDetails(physicalDevice->handle, surface);
    VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(supportDetails.formatsCount, supportDetails.formats);
    VkPresentModeKHR presentMode = selectPresentMode(supportDetails.presentModesCount, supportDetails.presentModes);
    VkExtent2D extent = selectSwapchainExtent(window, &supportDetails.capabilities);

    uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
    if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
        imageCount = supportDetails.capabilities.maxImageCount;
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = supportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    u32 indicesCount = 2;
    uint32_t indices[indicesCount] = {physicalDevice->queueFamilyIndices.graphicsQueue, physicalDevice->queueFamilyIndices.presentQueue};
    if (physicalDevice->queueFamilyIndices.graphicsQueue != physicalDevice->queueFamilyIndices.presentQueue){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = indicesCount;
        createInfo.pQueueFamilyIndices = indices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    SwapchainDetails swapchain = {};
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain.handle)){
        fprintf(stderr, "Failed to create Swapchain\n");
        exit(EXIT_FAILURE);
    }
    destroySwapchainSupportDetails(&supportDetails);

    swapchain.extent = extent;
    swapchain.format = surfaceFormat.format;

    vkGetSwapchainImagesKHR(device, swapchain.handle, &swapchain.imagesCount, NULL);
    swapchain.images = (VkImage*)malloc(sizeof(VkImage)*swapchain.imagesCount);
    vkGetSwapchainImagesKHR(device, swapchain.handle, &swapchain.imagesCount, swapchain.images);

    swapchain.imageViews = (VkImageView*)malloc(sizeof(VkImageView)*swapchain.imagesCount);
    for (size_t i = 0; i < swapchain.imagesCount; i++){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchain.images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, NULL, &swapchain.imageViews[i])){
            fprintf(stderr, "Failed to create Swapchain Image View %ld\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return swapchain;
}

void destroySwapchain(VkDevice device, SwapchainDetails *swapchain)
{
    for (size_t i = 0; i < swapchain->imagesCount; i++){
        vkDestroyImageView(device, swapchain->imageViews[i], NULL);
    }
    free(swapchain->imageViews);
    free(swapchain->images);
    vkDestroySwapchainKHR(device, swapchain->handle, NULL);
}