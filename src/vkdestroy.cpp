#include "vkdestroy.h"
#include <stdlib.h>

void destroyVkState(VkState *vkstate){
    vkDestroySwapchainKHR(vkstate->logicalDevice, vkstate->swapchain, NULL);
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