#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "vkinit.h"
#include "vkstate.h"
#include "vkdestroy.h"

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

    VkInstanceCreateInfo createInfo{};
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

bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device){
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extCount, NULL);
    VkExtensionProperties *extProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)*extCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extCount, extProperties);

    size_t remainingExtensions = DEVICE_EXTENSIONS_COUNT;
    for (size_t i = 0; i < extCount; i++){
        for (size_t j = 0; j < DEVICE_EXTENSIONS_COUNT; j++){
            if (!strcmp(extProperties[i].extensionName, DEVICE_EXTENSIONS[j])){
                remainingExtensions--;
                break;
            }
        }

        if (!remainingExtensions)
            break;
    }
    
    free(extProperties);

    if (remainingExtensions)
        return false;
    else
        return true;
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface){
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
        printf("\t%s\n", deviceProperties.deviceName);
    }
    #endif
    
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (size_t i = 0; i < deviceCount; i++){
        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevices[i], surface);
        if (indices.graphicsQueue < indices.queueFamilyCount &&
            indices.presentQueue < indices.queueFamilyCount &&
            checkPhysicalDeviceExtensionSupport(physicalDevices[i])
        ){
            SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevices[i], surface);
            if (swapchainSupport.formatsCount > 0 && swapchainSupport.presentModesCount > 0){
                selectedDevice = physicalDevices[i];
                destroySwapchainSupportDetails(&swapchainSupport);
                break;
            }
            destroySwapchainSupportDetails(&swapchainSupport);
        }
    }

    free(physicalDevices);

    #ifdef NDEBUG
    #else
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);
    printf("Selected GPU: %s\n", deviceProperties.deviceName);
    #endif

    if (!selectedDevice){
        printf("No GPU is suitable\n");
        exit(EXIT_FAILURE);
    }
    return selectedDevice;
}

QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface){
    QueueFamilyIndices indices{};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    indices.queueFamilyCount = queueFamilyCount;
    indices.graphicsQueue = queueFamilyCount;
    indices.presentQueue = queueFamilyCount;

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    
    for (size_t i = 0; i < queueFamilyCount; i++){
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsQueue = i;

        if (presentSupport)
            indices.presentQueue = i;
        
        //Prefer the queues to be the same
        if (indices.graphicsQueue < queueFamilyCount && 
            indices.presentQueue < queueFamilyCount &&
            indices.graphicsQueue == indices.presentQueue
        )
            break;
    }
    
    free(queueFamilies);
    return indices;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface){
    QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice, surface);

    uint32_t queueCreateInfoCount = 1;
    if (indices.graphicsQueue != indices.presentQueue){
        queueCreateInfoCount = 2;
        #ifdef NDEBUG
        #else
        printf("Graphics and Present Queue Families are different\n");
        #endif
    }
    VkDeviceQueueCreateInfo *queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo)*queueCreateInfoCount);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsQueue;//Expected to be available
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    queueCreateInfos[0] = queueCreateInfo;

    if (indices.presentQueue != indices.graphicsQueue){
        queueCreateInfo.queueFamilyIndex = indices.presentQueue;
        queueCreateInfos[1] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.enabledExtensionCount = DEVICE_EXTENSIONS_COUNT;
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
    createInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device)){
        printf("Failed to create Logical Device\n");
        exit(EXIT_FAILURE);
    }

    free(queueCreateInfos);
    return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window){
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, NULL, &surface)){
        printf("Failed to create Surface\n");
        exit(EXIT_FAILURE);
    }
    return surface;
}

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface){
    SwapchainSupportDetails details{};
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities)){
        printf("Failed to get Surface capabilities\n");
        exit(EXIT_FAILURE);
    }

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsCount, NULL);
    details.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR)*details.formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsCount, details.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesCount, NULL);
    details.presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR)*details.presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesCount, details.presentModes);

    return details;
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

SwapchainDetails createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow *window){
    SwapchainSupportDetails supportDetails = querySwapchainSupport(physicalDevice, surface);
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

    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(physicalDevice, surface);
    uint32_t indices[] = {queueFamilyIndices.graphicsQueue, queueFamilyIndices.presentQueue};
    if (queueFamilyIndices.graphicsQueue != queueFamilyIndices.presentQueue){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    SwapchainDetails swapchainDetails{};
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchainDetails.handle)){
        printf("Failed to create Swapchain\n");
        exit(EXIT_FAILURE);
    }
    destroySwapchainSupportDetails(&supportDetails);

    swapchainDetails.extent = extent;
    swapchainDetails.format = surfaceFormat.format;

    vkGetSwapchainImagesKHR(device, swapchainDetails.handle, &swapchainDetails.imagesCount, NULL);
    swapchainDetails.images = (VkImage*)malloc(sizeof(VkImage)*swapchainDetails.imagesCount);
    vkGetSwapchainImagesKHR(device, swapchainDetails.handle, &swapchainDetails.imagesCount, swapchainDetails.images);

    return swapchainDetails;
}