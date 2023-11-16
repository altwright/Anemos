#include "vkstate.h"
#include <stdio.h>
#include <assert.h>
#include "window.h"
#include "config.h"
#include "vkinstance.h"
#include "vkswapchain.h"
#include "vkdevice.h"

VkState initVkState(Window *window, UserConfig *config)
{
    VkState vk = {};
    vk.instance = createInstance(config);
    vk.surface = createSurface(vk.instance, window->handle);
    vk.physicalDevice = selectPhysicalDevice(vk.instance, vk.surface);
    vk.device = createLogicalDevice(&vk.physicalDevice);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.graphicsQueue, 0, &vk.graphicsQueue);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.presentQueue, 0, &vk.presentQueue);
    vk.swapchain = createSwapchain(vk.device, &vk.physicalDevice, vk.surface, window->handle);

    return vk;
}

void destroyVkState(VkState *vk)
{
    destroySwapchain(vk->device, &vk->swapchain);
    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
}