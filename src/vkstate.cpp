#include "vkstate.h"
#include <stdio.h>
#include <assert.h>
#include "window.h"
#include "config.h"
#include "vkinstance.h"
#include "vkswapchain.h"
#include "vkdevice.h"
#include "vkcommand.h"

VkState initVkState(Window *window, UserConfig *config)
{
    VkState vk = {};
    vk.instance = createInstance(config);
    vk.surface = createSurface(vk.instance, window->handle);
    vk.physicalDevice = selectPhysicalDevice(vk.instance, vk.surface);
    vk.device = createLogicalDevice(&vk.physicalDevice);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.graphicsQueue, 0, &vk.graphicsQueue);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.presentQueue, 0, &vk.presentQueue);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.transferQueue, 0, &vk.transferQueue);
    vk.swapchain = createSwapchain(vk.device, &vk.physicalDevice, vk.surface, window->handle);
    vk.graphicsCommandPool = createCommandPool(vk.device, vk.physicalDevice.queueFamilyIndices.graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    vk.transferCommandPool = createCommandPool(vk.device, vk.physicalDevice.queueFamilyIndices.graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    return vk;
}

void destroyVkState(VkState *vk)
{
    vkDestroyCommandPool(vk->device, vk->transferCommandPool, NULL);
    vkDestroyCommandPool(vk->device, vk->graphicsCommandPool, NULL);
    destroySwapchain(vk->device, &vk->swapchain);
    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
}