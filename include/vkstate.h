#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "int.h"
#include "window.h"
#include "config.h"

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
} FrameSynchronisers;

typedef struct {
    VkBuffer handle;
    VmaAllocation alloc;
    VmaAllocationInfo info;
} Buffer;

typedef struct {
    VkPhysicalDevice handle;
    VkPhysicalDeviceMemoryProperties memProperties;
    VkPhysicalDeviceProperties deviceProperties;
    QueueFamilyIndices queueFamilyIndices;
    VkSampleCountFlagBits maxSamplingCount;
} PhysicalDeviceDetails;

typedef struct {
    VkDescriptorSet handle;
    Buffer buffer;
    void* mappedBuffer;
} DescriptorSet;

typedef struct {
    DescriptorSet *sets;//free
    size_t setsCount;
    VkDescriptorSetLayout layout;
} Descriptors;

typedef struct {
    VkImage handle;
    VmaAllocation alloc;
    VkImageView view;
    VkExtent2D extent;
    VkFormat format;
} Image;

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
    Image depthImage;
    Image samplingImage;
    VkRenderPass renderPass;
    Framebuffers framebuffers;
    PipelineDetails graphicsPipeline;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transferCommandPool;
    FrameSynchronisers frameSyncers[MAX_FRAMES_IN_FLIGHT];
    Buffer deviceBuffer;
    Buffer stagingBuffer;
} VulkanState;

VulkanState initVulkanState(Window *window, const UserConfig *config);
void destroyVulkanState(VulkanState *vk);