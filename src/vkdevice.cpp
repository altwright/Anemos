#include "vkdevice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vkswapchain.h"

QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface){
    QueueFamilyIndices indices = {};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &indices.queueFamilyCount, NULL);
    indices.graphicsQueue = indices.queueFamilyCount;
    indices.presentQueue = indices.queueFamilyCount;
    indices.transferQueue = indices.queueFamilyCount;

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*indices.queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &indices.queueFamilyCount, queueFamilies);
    
    for (size_t i = 0; i < indices.queueFamilyCount; i++){
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsQueue = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (presentSupport && indices.presentQueue == indices.queueFamilyCount)
            indices.presentQueue = i;
        
        //Prefer the queues to be the same
        if (indices.graphicsQueue < indices.queueFamilyCount && 
            indices.presentQueue < indices.queueFamilyCount &&
            indices.graphicsQueue == indices.presentQueue)
            break;
    }

    for (size_t i = 0; i < indices.queueFamilyCount; i++){
        if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && i != indices.graphicsQueue){
            indices.transferQueue = i;
            break;
        }
    }
    
    if (indices.transferQueue == indices.queueFamilyCount)
        indices.transferQueue = indices.graphicsQueue;//Guaranteed to support transfer

    free(queueFamilies);

    return indices;
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

VkFormat findSupportedFormat(
    VkPhysicalDevice device, 
    VkFormat *candidateFormats, 
    size_t candidateFormatsCount,
    VkImageTiling tiling,
    VkFormatFeatureFlags desiredFeatures)
{
    for (size_t i = 0; i < candidateFormatsCount; i++){
        VkFormatProperties formatProperties = {};
        vkGetPhysicalDeviceFormatProperties(device, candidateFormats[i], &formatProperties);

        if (tiling == VK_IMAGE_TILING_LINEAR && 
            (formatProperties.linearTilingFeatures & desiredFeatures) == desiredFeatures){
            return candidateFormats[i];
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && 
            (formatProperties.optimalTilingFeatures & desiredFeatures) == desiredFeatures){
            return candidateFormats[i];
        }
    }

    return VK_FORMAT_MAX_ENUM;
}

bool checkPhysicalDeviceFeatureSupport(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures supportedFeatures = {};
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    size_t candidateDepthBufferFormatsCount = 3;
    VkFormat candidateDepthBufferFormats[candidateDepthBufferFormatsCount] = {
        VK_FORMAT_D32_SFLOAT, 
        VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT};
    VkFormat depthBufferFormat = findSupportedFormat(
        physicalDevice, 
        candidateDepthBufferFormats, 
        candidateDepthBufferFormatsCount, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Change this to a broader feature search
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilLayouts = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES};
    VkPhysicalDeviceFeatures2 supportedFeatures2 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    supportedFeatures2.pNext = &separateDepthStencilLayouts;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &supportedFeatures2);
    
    return supportedFeatures.samplerAnisotropy && 
        (depthBufferFormat != VK_FORMAT_MAX_ENUM) && 
        separateDepthStencilLayouts.separateDepthStencilLayouts;
}

VkPhysicalDevice selectFromPhysicalDevices(
    VkPhysicalDevice *physicalDevices, 
    size_t physicalDevicesCount, 
    VkSurfaceKHR surface, 
    VkPhysicalDeviceType preferredDeviceType)
{
    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (size_t i = 0; i < physicalDevicesCount; i++)
    {
        bool suitableFound = false;

        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        if (properties.deviceType != preferredDeviceType){
            continue;
        }

        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevices[i], surface);

        if (indices.graphicsQueue < indices.queueFamilyCount &&
            indices.presentQueue < indices.queueFamilyCount &&
            checkPhysicalDeviceExtensionSupport(physicalDevices[i]) &&
            checkPhysicalDeviceFeatureSupport(physicalDevices[i]))
        {
            SwapchainSupportDetails swapchainSupport = querySwapchainSupportDetails(physicalDevices[i], surface);
            if (swapchainSupport.formatsCount > 0 && swapchainSupport.presentModesCount > 0){
                selectedDevice = physicalDevices[i];
                suitableFound = true;
            }
            destroySwapchainSupportDetails(&swapchainSupport);
        }

        if (suitableFound)
            break;
    }

    return selectedDevice;
}

PhysicalDeviceDetails selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, NULL);
    if (!physicalDevicesCount){
        fprintf(stderr, "Failed to find Vulkan-capable GPU\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*physicalDevicesCount);
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices);

    #ifndef NDEBUG
    printf("Vulkan-capable GPUs:\n");
    for (size_t i = 0; i < physicalDevicesCount; i++){
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
        printf("\t%s\n", deviceProperties.deviceName);
    }
    #endif
    
    VkPhysicalDevice selectedDevice = selectFromPhysicalDevices(
        physicalDevices,
        physicalDevicesCount,
        surface,
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    );

    if (!selectedDevice){
        selectedDevice = selectFromPhysicalDevices(
            physicalDevices,
            physicalDevicesCount,
            surface,
            VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        );
    }

    free(physicalDevices);

    if (!selectedDevice){
        fprintf(stderr, "No GPU is suitable\n");
        exit(EXIT_FAILURE);
    }

    PhysicalDeviceDetails physicalDeviceDetails = {};
    physicalDeviceDetails.handle = selectedDevice;
    vkGetPhysicalDeviceMemoryProperties(physicalDeviceDetails.handle, &physicalDeviceDetails.memProperties);
    vkGetPhysicalDeviceProperties(physicalDeviceDetails.handle, &physicalDeviceDetails.deviceProperties);
    physicalDeviceDetails.queueFamilyIndices = findQueueFamilyIndices(physicalDeviceDetails.handle, surface);

    #ifndef NDEBUG
    printf("Selected GPU: %s\n", &physicalDeviceDetails.deviceProperties.deviceName);
    #endif

    VkSampleCountFlags maxMSAA = physicalDeviceDetails.deviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceDetails.deviceProperties.limits.framebufferDepthSampleCounts;

    if (maxMSAA & VK_SAMPLE_COUNT_64_BIT){
        physicalDeviceDetails.maxMSAA = VK_SAMPLE_COUNT_64_BIT;
    }
    else if (maxMSAA & VK_SAMPLE_COUNT_32_BIT){
        physicalDeviceDetails.maxMSAA = VK_SAMPLE_COUNT_32_BIT;
    }
    else if (maxMSAA & VK_SAMPLE_COUNT_16_BIT){
        physicalDeviceDetails.maxMSAA = VK_SAMPLE_COUNT_16_BIT;
    }
    else if (maxMSAA & VK_SAMPLE_COUNT_8_BIT){
        physicalDeviceDetails.maxMSAA = VK_SAMPLE_COUNT_8_BIT;
    }
    else {
        fprintf(stderr, "Failed to find a suitable Max Sample Count\n");
        exit(EXIT_FAILURE);
    }

    return physicalDeviceDetails;
}

VkDevice createLogicalDevice(const PhysicalDeviceDetails *physicalDevice)
{
    uint32_t queueCreateInfoCount = 1;
    if (physicalDevice->queueFamilyIndices.graphicsQueue != physicalDevice->queueFamilyIndices.presentQueue){
        queueCreateInfoCount++;
        #ifndef NDEBUG
        printf("Graphics and Present Queue Families are different\n");
        #endif
    }
    if (physicalDevice->queueFamilyIndices.transferQueue != physicalDevice->queueFamilyIndices.graphicsQueue){
        queueCreateInfoCount++;
    }
    else {
        #ifndef NDEBUG
        printf("Graphics and Transfer Queue Families are the same\n");
        #endif
    }

    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = physicalDevice->queueFamilyIndices.graphicsQueue;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceQueueCreateInfo *queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo)*queueCreateInfoCount);
    queueCreateInfos[0] = queueCreateInfo;
    queueCreateInfoCount = 1;
    if (physicalDevice->queueFamilyIndices.presentQueue != physicalDevice->queueFamilyIndices.graphicsQueue){
        queueCreateInfoCount++;
        queueCreateInfo.queueFamilyIndex = physicalDevice->queueFamilyIndices.presentQueue;
        queueCreateInfos[queueCreateInfoCount-1] = queueCreateInfo;
    }
    if (physicalDevice->queueFamilyIndices.transferQueue != physicalDevice->queueFamilyIndices.graphicsQueue){
        queueCreateInfoCount++;
        queueCreateInfo.queueFamilyIndex = physicalDevice->queueFamilyIndices.transferQueue;
        queueCreateInfos[queueCreateInfoCount-1] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.enabledExtensionCount = DEVICE_EXTENSIONS_COUNT;
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
    createInfo.pEnabledFeatures = &deviceFeatures;

    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilFeature = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES};
    separateDepthStencilFeature.separateDepthStencilLayouts = VK_TRUE;

    createInfo.pNext = &separateDepthStencilFeature;

    VkDevice device;
    if (vkCreateDevice(physicalDevice->handle, &createInfo, NULL, &device)){
        printf("Failed to create Logical Device\n");
        exit(EXIT_FAILURE);
    }

    free(queueCreateInfos);

    return device;
}