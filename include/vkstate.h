#pragma once
#include <vulkan/vulkan.h>

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

typedef struct QueueFamilyIndices{
    bool graphicsQueuePresent;
    uint32_t graphicsQueue;
} QueueFamilyIndices;

typedef struct VkState{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices queueFamilyIndices;
} VkState;
