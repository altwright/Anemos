#pragma once
#include <vulkan/vulkan.h>
#include "int.h"
#include "window.h"
#include "config.h"

typedef struct QueueFamilyIndices{
    u32 queueFamilyCount;
    u32 graphicsQueue;
    u32 presentQueue;
    u32 transferQueue;
} QueueFamilyIndices;

typedef struct SwapchainDetails{
    VkSwapchainKHR handle;
    u32 imagesCount;
    VkImage *images;//free
    VkImageView *imageViews;//free
    VkFormat format;
    VkExtent2D extent;
} SwapchainDetails;

typedef struct PipelineDetails{
    VkPipeline handle;
    VkPipelineLayout layout;
} PipelineDetails;

typedef struct Framebuffers{
    size_t count;
    VkFramebuffer *handles;//free
} Framebuffers;

typedef struct Synchronisers{
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlight;
} Synchronisers;

typedef struct FrameControllers{
    VkCommandBuffer commandBuffer;
    Synchronisers synchronisers;
} FrameControllers;

typedef struct Buffer{
    VkBuffer handle;
    VkDeviceSize size;
    VkDeviceMemory memory;
    VkDeviceSize physicalSize;
} Buffer;

typedef struct PhysicalDeviceDetails{
    VkPhysicalDevice handle;
    VkPhysicalDeviceMemoryProperties memProperties;
    VkPhysicalDeviceProperties deviceProperties;
    QueueFamilyIndices queueFamilyIndices;
    VkSampleCountFlagBits maxMSAA;
} PhysicalDeviceDetails;

typedef struct DescriptorSet{
    VkDescriptorSet handle;
    Buffer buffer;
    void* mappedBuffer;
} DescriptorSet;

typedef struct Descriptors{
    DescriptorSet *sets;//free
    size_t setsCount;
    VkDescriptorSetLayout layout;
} Descriptors;

typedef struct Image{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    VkExtent2D extent;
    VkFormat format;
} Image;

typedef struct VkState{
    VkInstance instance;
    VkSurfaceKHR surface;
    PhysicalDeviceDetails physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    SwapchainDetails swapchain;
    VkRenderPass renderPass;
    VkDescriptorPool descriptorPool;
    Descriptors descriptors;
    PipelineDetails graphicsPipeline;
    Framebuffers framebuffers;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transferCommandPool;
    FrameControllers *frameControllers;//free
    Image texture;
    VkSampler textureSampler;
    Image depthBuffer;
    Image sampleImage;
} VkState;

VkState initVkState(Window *window, UserConfig *config);
void destroyVkState(VkState *vk);