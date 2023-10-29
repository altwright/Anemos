#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vkinit.h"
#include "vkstate.h"

VkInstance createInstance(const char *appName, uint32_t appVersion, const char *engineName, uint32_t engineVersion){
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = appName,
        .applicationVersion = appVersion,
        .pEngineName = engineName,
        .engineVersion = engineVersion,
        .apiVersion = VK_API_VERSION_1_3
    };

    //Get essential extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //Get supported extensions
    uint32_t supportedExtCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &supportedExtCount, NULL);
    VkExtensionProperties *extProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)*supportedExtCount);
    vkEnumerateInstanceExtensionProperties(NULL, &supportedExtCount, extProperties);
    #ifdef NDEBUG
    #else
    printf("Supported Instance Extensions:\n");
    for(size_t i = 0; i < supportedExtCount; i++){
        printf("\t%s\n", extProperties[i].extensionName);
    }
    #endif

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    
    #ifdef NDEBUG
    createInfo.enabledLayerCount = 0;
    #else
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *layerProperties = (VkLayerProperties*)malloc(sizeof(VkLayerProperties)*layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties);
    for (size_t i = 0; i < VALIDATION_LAYERS_COUNT; i++){
        bool layerFound = false;
        for (size_t j = 0; j < layerCount; j++){
            if (!strcmp(VALIDATION_LAYERS[i], layerProperties[j].layerName)){
                layerFound = true;
                break;
            }
        }

        if (!layerFound){
            printf("Could not find %s\n", VALIDATION_LAYERS[i]);
            exit(EXIT_FAILURE);
        }
    }

    createInfo.enabledLayerCount = VALIDATION_LAYERS_COUNT;
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
    free(layerProperties);
    #endif

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance)){
        printf("Failed to create Vulkan Instance\n");
        exit(EXIT_FAILURE);
    }

    free(extProperties);
    return instance;
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    if (!deviceCount){
        printf("Failed to find Vulkan-capable GPU\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    #ifdef NDEBUG
    #else
    printf("Vulkan-capable GPUs:\n");
    for (size_t i = 0; i < deviceCount; i++){
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);
        printf("\t%s\n", deviceProperties.deviceName);
    }
    #endif
    
    //Select physical device
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (size_t i = 0; i < deviceCount; i++){
        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevices[i]);
        if (indices.graphicsQueue < indices.queueFamilyCount){
            selectedDevice = physicalDevices[i];
            #ifdef NDEBUG
            #else
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);
            printf("Selected GPU: %s\n", deviceProperties.deviceName);
            #endif
            break;
        }
    }
    free(physicalDevices);

    if (!selectedDevice){
        printf("No GPU is suitable\n");
        exit(EXIT_FAILURE);
    }
    return selectedDevice;
}

struct QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device){
    QueueFamilyIndices indices = {};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    indices.queueFamilyCount = queueFamilyCount;
    indices.graphicsQueue = queueFamilyCount;

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    
    for (size_t i = 0; i < queueFamilyCount; i++){
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsQueue = i;
        }
    }
    
    free(queueFamilies);
    return indices;
}