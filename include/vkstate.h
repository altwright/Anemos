#pragma once
#include <vulkan/vulkan.h>

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

#define DEVICE_EXTENSIONS_COUNT 1
extern const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT];

typedef struct SwapchainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    size_t formatsLen;
    VkSurfaceFormatKHR *formats;
    size_t presentModesLen;
    VkPresentModeKHR *presentModes;
} SwapchainSupportDetails;

typedef struct QueueFamilyIndices{
    uint32_t queueFamilyCount;
    uint32_t graphicsQueue;
    uint32_t presentQueue;
} QueueFamilyIndices;

typedef struct VkState{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
} VkState;
