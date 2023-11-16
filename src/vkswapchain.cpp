#include "vkswapchain.h"
#include <stdlib.h>
#include <stdio.h>

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