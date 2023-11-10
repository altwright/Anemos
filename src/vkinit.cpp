#define GLFW_INCLUDE_VULKAN
#include "vkinit.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "vkstate.h"
#include "vkdestroy.h"
#include "load.h"
#include "window.h"
#include "vertex.h"
#include "vkmemory.h"
#include "vkdescriptor.h"
#include "stb_image.h"
#include "vkimage.h"

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

PhysicalDeviceDetails selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface){
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
        bool suitableFound = false;
        if (indices.graphicsQueue < indices.queueFamilyCount &&
            indices.presentQueue < indices.queueFamilyCount &&
            checkPhysicalDeviceExtensionSupport(physicalDevices[i])
        ){
            SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevices[i], surface);
            if (swapchainSupport.formatsCount > 0 && swapchainSupport.presentModesCount > 0){
                selectedDevice = physicalDevices[i];
                suitableFound = true;
            }
            destroySwapchainSupportDetails(&swapchainSupport);
        }

        if (suitableFound)
            break;
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

    PhysicalDeviceDetails physicalDeviceDetails = {};
    physicalDeviceDetails.handle = selectedDevice;
    vkGetPhysicalDeviceMemoryProperties(physicalDeviceDetails.handle, &physicalDeviceDetails.memProperties);

    return physicalDeviceDetails;
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

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices){
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

    swapchainDetails.imageViews = (VkImageView*)malloc(sizeof(VkImageView)*swapchainDetails.imagesCount);
    for (size_t i = 0; i < swapchainDetails.imagesCount; i++){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainDetails.images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchainDetails.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, NULL, &swapchainDetails.imageViews[i])){
            printf("Failed to create Swapchain Image View %ld\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return swapchainDetails;
}

VkShaderModule createShaderModule(VkDevice device, uint32_t *code, size_t numBytes){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = numBytes;
    createInfo.pCode = code;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, NULL, &module)){
        printf("Failed to create Shader Module of byte size %ld\n", numBytes);
        exit(EXIT_FAILURE);
    }

    return module;
}

VkRenderPass createRenderPass(VkDevice device, const SwapchainDetails *swapchainDetails){
    VkAttachmentDescription colourAttachmentDesc{};
    colourAttachmentDesc.format = swapchainDetails->format;
    colourAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    colourAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //Subpasses are subsequent rendering operations that depend on the contents of 
    //framebuffers in previous passes

    VkAttachmentReference colourAttachmentRef{};
    colourAttachmentRef.attachment = 0;//References the first attachment description by index
    colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //Transitions layout of attachment to this during subpass

    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colourAttachmentRef;
    //The index of the attachment in this array is directly 
    //referenced from the fragment shader with the 
    //layout(location = 0) out vec4 outColor directive!

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;//Index to the only subpass. Must be higher than srcSubpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    //The above two fields specify the operations to wait on and the stages in which 
    //these operations occur. We need to wait for the swap chain to finish reading 
    //from the image before we can access it. This can be accomplished by waiting on 
    //the color attachment output stage itself.
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //The operations that should wait on this are in the color attachment stage 
    //and involve the writing of the color attachment. These settings will prevent 
    //the transition from happening until it’s actually necessary (and allowed): 
    //when we want to start writing colors to it.
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colourAttachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass)){
        printf("Failed to create Render Pass\n");
        exit(EXIT_FAILURE);
    }

    return renderPass;
}

Descriptors createDescriptors(
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkDescriptorPool descriptorPool,
    size_t numFramesInFlight)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    Descriptors descriptors = {};
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptors.layout)){
        fprintf(stderr, "Failed to create Descriptor Set Layout\n");
        exit(EXIT_FAILURE);
    }

    descriptors.setsCount = numFramesInFlight;
    descriptors.sets = (DescriptorSet*)malloc(sizeof(DescriptorSet)*descriptors.setsCount);
    if (!descriptors.sets){
        fprintf(stderr, "Failed to allocate sets for Descriptors\n");
        exit(EXIT_FAILURE);
    }

    VkDescriptorSetLayout *layouts = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout)*descriptors.setsCount);//free
    if (!layouts){
        fprintf(stderr, "Failed to allocate Descriptor Set Layout array\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < descriptors.setsCount; i++){
        descriptors.sets[i].buffer = createBuffer(
            device,
            physicalDevice,
            sizeof(UniformBufferData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        vkMapMemory(
            device,
            descriptors.sets[i].buffer.memory,
            0,
            descriptors.sets[i].buffer.size,
            0,
            &descriptors.sets[i].mappedBuffer
        );

        layouts[i] = descriptors.layout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = descriptors.setsCount;
    allocInfo.pSetLayouts = layouts;

    VkDescriptorSet *descriptorSets = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet)*descriptors.setsCount);//free
    if (!descriptorSets){
        fprintf(stderr, "Failed to malloc Descriptor Sets array\n");
        exit(EXIT_FAILURE);
    }

    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets)){
        fprintf(stderr, "Failed to allocate Descriptor Sets\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < descriptors.setsCount; i++){
        descriptors.sets[i].handle = descriptorSets[i];

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = descriptors.sets[i].buffer.handle;
        bufferInfo.offset = 0;
        bufferInfo.range = descriptors.sets[i].buffer.size;

        VkWriteDescriptorSet writeDescriptorUpdate = {};
        writeDescriptorUpdate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorUpdate.dstSet = descriptors.sets[i].handle;
        writeDescriptorUpdate.dstBinding = 0;
        //Remember that descriptors can be arrays, so we also need to specify 
        //the first index in the array that we want to update
        writeDescriptorUpdate.dstArrayElement = 0;
        writeDescriptorUpdate.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //It’s possible to update multiple descriptors at once in an array, 
        //starting at index dstArrayElement. The descriptorCount field specifies 
        //how many array elements you want to update.
        writeDescriptorUpdate.descriptorCount = 1;
        //The pBufferInfo field is used for descriptors that refer to buffer data, 
        //pImageInfo is used for descriptors that refer to image data, and pTexelBufferView 
        //is used for descriptors that refer to buffer views.
        writeDescriptorUpdate.pBufferInfo = &bufferInfo;
        writeDescriptorUpdate.pImageInfo = nullptr; // Optional
        writeDescriptorUpdate.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &writeDescriptorUpdate, 0, NULL);
    }

    free(descriptorSets);
    free(layouts);
    return descriptors;
}

PipelineDetails createGraphicsPipeline(
    VkDevice device, 
    VkRenderPass renderPass, 
    const SwapchainDetails *swapchainDetails,
    Descriptors descriptors)
{
    #ifdef NDEBUG
    FileContents vertShader = readFileContents("shaders/vert.spv");
    FileContents fragShader = readFileContents("shaders/frag.spv");
    #else
    FileContents vertShader = readFileContents("../shaders/vert.spv");
    FileContents fragShader = readFileContents("../shaders/frag.spv");
    #endif

    if (!vertShader.bytes){
        printf("Failed to find the Vertex Shader binary\n");
        exit(EXIT_FAILURE);
    }

    if (!fragShader.bytes){
        printf("Failed to find the Fragment Shader binary\n");
        exit(EXIT_FAILURE);
    }

    VkShaderModule vertShaderModule = createShaderModule(device, (uint32_t*)vertShader.bytes, vertShader.len);
    VkShaderModule fragShaderModule = createShaderModule(device, (uint32_t*)fragShader.bytes, fragShader.len);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkVertexInputBindingDescription bindingDesc = getVertexBindingDescription();
    VertexInputAttributes attributes = getVertexInputAttributes();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = attributes.count;
    vertexInputInfo.pVertexAttributeDescriptions = attributes.descriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;

    VkPipelineViewportStateCreateInfo viewportInfo{};//Both viewport and scissor will be dynamic
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.scissorCount = 1;
    viewportInfo.viewportCount = 1;
    //Actual viewport and scissor will be set up at drawing time.

    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;//Requires a GPU feature
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;//Requires a GPU feature for any other mode
    rasterizationInfo.lineWidth = 1.0f;//Any thicker lines requires the widelines GPU feature
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.minSampleShading = 1.0f; // Optional
    multisamplingInfo.pSampleMask = nullptr; // Optional
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};//Per framebuffer struct
    colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentInfo.blendEnable = VK_FALSE;
    colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachmentInfo;
    colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};//For specifying uniform variables
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; 
    pipelineLayoutInfo.pSetLayouts = &descriptors.layout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout)){
        printf("Failed to create Pipeline Layout\n");
        exit(EXIT_FAILURE);
    }

    VkPipelineShaderStageCreateInfo shaderStages[2] = {vertShaderStageInfo, fragShaderStageInfo};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    //It is possible to use other compatible render passes
    pipelineInfo.subpass = 0;//Index of the subpass where this graphics pipeline will be used

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline)){
        printf("Failed to create sole Graphics Pipeline\n");
        exit(EXIT_FAILURE);
    }

    PipelineDetails pipelineDetails = {
        .handle = pipeline,
        .layout = pipelineLayout
    };

    free(vertShader.bytes);
    free(fragShader.bytes);
    free(attributes.descriptions);
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);

    return pipelineDetails;
}

Framebuffers createFramebuffers(VkDevice device, VkRenderPass renderPass, const SwapchainDetails *swapchainDetails){
    Framebuffers framebuffers{};
    framebuffers.count = swapchainDetails->imagesCount;
    framebuffers.handles = (VkFramebuffer*)malloc(sizeof(VkFramebuffer)*framebuffers.count);

    for(size_t i = 0; i < framebuffers.count; i++){
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        //You can only use a framebuffer with the render passes that it is compatible 
        //with, which roughly means that they use the same number and type of attachments.
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &swapchainDetails->imageViews[i];
        //The attachmentCount and pAttachments parameters specify the VkImageView objects 
        //that should be bound to the respective attachment descriptions in the render pass pAttachment array.
        framebufferInfo.width = swapchainDetails->extent.width;
        framebufferInfo.height = swapchainDetails->extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &framebuffers.handles[i])){
            printf("Failed to create Framebuffer %ld\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return framebuffers;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags createFlags){
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = createFlags;
    poolInfo.queueFamilyIndex = queueIndex;

    VkCommandPool pool;
    if (vkCreateCommandPool(device, &poolInfo, NULL, &pool)){
        printf("Failed to create Command Pool\n");
        exit(EXIT_FAILURE);
    }

    return pool;
}

VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool commandPool){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)){
        printf("Failed to create Command Buffer\n");
        exit(EXIT_FAILURE);
    }

    return commandBuffer;
}

Synchronisers createSynchronisers(VkDevice device){
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;//Starts in the signalled state

    Synchronisers synchronisers{};
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &synchronisers.imageAvailable) ||
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &synchronisers.renderFinished) ||
        vkCreateFence(device, &fenceInfo, NULL, &synchronisers.inFlight)){
        printf("Failed to create Synchronizer members\n");
        exit(EXIT_FAILURE);
    }

    return synchronisers;
}

FrameControllers* createFrameStates(VkDevice device, VkCommandPool commandPool, size_t numFrames){
    FrameControllers *frameStates = (FrameControllers*)malloc(sizeof(FrameControllers)*numFrames);
    if (!frameStates){
        printf("Failed to allocated FrameStates\n");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < numFrames; i++){
        frameStates[i].commandBuffer = createCommandBuffer(device, commandPool);
        frameStates[i].synchronisers = createSynchronisers(device);
    }

    return frameStates;
}

void recreateSwapchain(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkRenderPass renderPass,
    GLFWwindow *window,
    SwapchainDetails *swapchainDetails,
    Framebuffers *framebuffers)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0){
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    destroySwapchainDetails(device, swapchainDetails);
    *swapchainDetails = createSwapchain(device, physicalDevice, surface, window);

    destroyFramebuffers(device, framebuffers);
    *framebuffers = createFramebuffers(device, renderPass, swapchainDetails);
}

VkDescriptorPool createDescriptorPool(VkDevice device, u32 numFramesInFlight)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = numFramesInFlight;//Max descriptors

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = numFramesInFlight;

    VkDescriptorPool descriptorPool = NULL;
    if (vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool)){
        fprintf(stderr, "Failed to create Descriptor Pool\n");
        exit(EXIT_FAILURE);
    }

    return descriptorPool;
}

Image createTexture(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDeviceDetails,
    VkCommandPool transientCommandPool,
    VkQueue transferQueue)
{
    int width, height, fileChannels = 0;

    stbi_uc *texels = stbi_load(
        "../textures/texture.jpg", 
        &width, 
        &height, 
        &fileChannels,
        STBI_rgb_alpha
    );

    if (!texels){
        fprintf(stderr, "Failed to load Texture Pixels\n");
        exit(EXIT_FAILURE);
    }

    Image texture = createImage(
        device, 
        &physicalDeviceDetails->memProperties,
        width,
        height,
        STBI_rgb_alpha,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transitionImageLayout(
        device,
        transientCommandPool,
        transferQueue,
        texture.handle,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyPixelsToLocalImage(
        device,
        physicalDeviceDetails,
        transientCommandPool,
        transferQueue,
        texels,
        sizeof(stbi_uc)*STBI_rgb_alpha,
        texture.height,
        texture.width,
        texture.handle);

    transitionImageLayout(
        device,
        transientCommandPool,
        transferQueue,
        texture.handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    stbi_image_free(texels);

    return texture;
}

VkState initVkState(const Window *window){
    VkState vk{};
    vk.instance = createInstance("Anemos", VK_MAKE_VERSION(0, 1, 0), "Moebius", VK_MAKE_VERSION(0, 1, 0));
    vk.surface = createSurface(vk.instance, window->handle);
    vk.physicalDevice = selectPhysicalDevice(vk.instance, vk.surface);
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(vk.physicalDevice.handle, vk.surface);
    vk.device = createLogicalDevice(vk.physicalDevice.handle, queueFamilyIndices);
    vkGetDeviceQueue(vk.device, queueFamilyIndices.graphicsQueue, 0, &vk.graphicsQueue);
    vkGetDeviceQueue(vk.device, queueFamilyIndices.presentQueue, 0, &vk.presentQueue);
    vk.swapchain = createSwapchain(vk.device, vk.physicalDevice.handle, vk.surface, window->handle);
    vk.renderPass = createRenderPass(vk.device, &vk.swapchain);
    /*
    The descriptor set layout specifies the types of resources that are going to be accessed 
    by the pipeline, just like a render pass specifies the types of attachments that will be 
    accessed. A descriptor set specifies the actual buffer or image resources that will be bound 
    to the descriptors, just like a framebuffer specifies the actual image views to bind to render 
    pass attachments. The descriptor set is then bound for the drawing commands just like the vertex 
    buffers and framebuffer.
    */
    vk.descriptorPool = createDescriptorPool(vk.device, MAX_FRAMES_IN_FLIGHT);
    vk.descriptors = createDescriptors(vk.device, &vk.physicalDevice, vk.descriptorPool, MAX_FRAMES_IN_FLIGHT);
    vk.graphicsPipeline = createGraphicsPipeline(vk.device, vk.renderPass, &vk.swapchain, vk.descriptors);
    vk.framebuffers = createFramebuffers(vk.device, vk.renderPass, &vk.swapchain);
    vk.vertexBuffer = createBuffer(
        vk.device,
        &vk.physicalDevice,
        sizeof(Vertex)*VERTEX_COUNT,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    vk.indexBuffer = createBuffer(
        vk.device,
        &vk.physicalDevice, 
        sizeof(indices[0])*INDEX_COUNT,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    vk.graphicsCommandPool = createCommandPool(vk.device, queueFamilyIndices.graphicsQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk.transferCommandPool = createCommandPool(vk.device, queueFamilyIndices.graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    vk.frameContollers = createFrameStates(vk.device, vk.graphicsCommandPool, MAX_FRAMES_IN_FLIGHT);
    vk.texture = createTexture(vk.device, &vk.physicalDevice, vk.transferCommandPool, vk.graphicsQueue);

    return vk;
}