#include "vkdestroy.h"
#include <stdlib.h>

void destroyVkState(VkState *vkstate){
    free(vkstate->swapchain.images);
    vkDestroySwapchainKHR(vkstate->logicalDevice, vkstate->swapchain.handle, NULL);
    vkDestroyDevice(vkstate->logicalDevice, NULL);
    vkDestroySurfaceKHR(vkstate->instance, vkstate->surface, NULL);
    vkDestroyInstance(vkstate->instance, NULL);
}

void destroySwapchainSupportDetails(SwapchainSupportDetails *details){
    if (details->formats)
        free(details->formats);
    if (details->presentModes)
        free(details->presentModes);
}