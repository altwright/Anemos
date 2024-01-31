#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "int.h"
#include "window.h"
#include "config.h"
#include "vkmemory.h"

typedef struct {
    u32 queueFamilyCount;
    u32 graphicsQueue;
    u32 presentQueue;
    u32 transferQueue;
} QueueFamilyIndices;

typedef struct {
    VkSwapchainKHR handle;
    u32 imagesCount;
    VkImage *images;//free
    VkImageView *imageViews;//free
    VkFormat format;
    VkExtent2D extent;
} SwapchainDetails;

typedef struct {
    VkPipeline handle;
    VkPipelineLayout layout;
} PipelineDetails;

typedef struct {
    size_t count;
    VkFramebuffer *handles;//free
} Framebuffers;

typedef struct {
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlight;
} FrameSynchroniser;

typedef struct {
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties properties;
    QueueFamilyIndices queueFamilyIndices;
    VkSampleCountFlagBits maxSamplingCount;
} PhysicalDeviceDetails;

typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;
    PhysicalDeviceDetails physicalDevice;
    VkDevice device;
    VmaAllocator allocator;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    SwapchainDetails swapchain;
    DeviceImage depthImage;
    DeviceImage samplingImage;
    VkRenderPass renderPass;
    Framebuffers framebuffers;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    PipelineDetails graphicsPipeline;
    VkCommandPool graphicsCmdPools[MAX_FRAMES_IN_FLIGHT];
    VkCommandPool transferCommandPool;
    FrameSynchroniser frameSyncers[MAX_FRAMES_IN_FLIGHT];
    VkSampler sampler;

    Buffer deviceBuffer;
    Buffer stagingBuffer;
    Buffer uniformBuffer;
} VulkanState;

VulkanState initVulkanState(Window *window, const UserConfig *config);
void destroyVulkanState(VulkanState *vk);