#pragma once
#include <vulkan/vulkan.h>

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

#define DEVICE_EXTENSIONS_COUNT 1
extern const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT];

/**
 * @brief Must call destroySwapchainSupportDetails() after
 */
typedef struct SwapchainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatsCount;
    VkSurfaceFormatKHR *formats;//free
    uint32_t presentModesCount;
    VkPresentModeKHR *presentModes;//free
} SwapchainSupportDetails;

typedef struct QueueFamilyIndices{
    uint32_t queueFamilyCount;
    uint32_t graphicsQueue;
    uint32_t presentQueue;
} QueueFamilyIndices;

typedef struct SwapchainDetails{
    VkSwapchainKHR handle;
    uint32_t imagesCount;
    VkImage *images;//free
    VkImageView *imageViews;//free
    VkFormat format;
    VkExtent2D extent;
} SwapchainDetails;

typedef struct PipelineDetails{
    VkPipeline handle;
    VkPipelineLayout layout;
} PipelineDetails;

typedef struct VkState{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    SwapchainDetails swapchain;
    VkRenderPass renderPass;
    PipelineDetails pipeline;
} VkState;
