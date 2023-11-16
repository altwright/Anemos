#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatsCount;
    VkSurfaceFormatKHR *formats;//free
    uint32_t presentModesCount;
    VkPresentModeKHR *presentModes;//free
} SwapchainSupportDetails;

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);
SwapchainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
void destroySwapchainSupportDetails(SwapchainSupportDetails *details);