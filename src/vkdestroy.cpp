#include "vkdestroy.h"

void destroyVkState(VkState *vkstate){
    vkDestroyDevice(vkstate->logicalDevice, NULL);
    vkDestroySurfaceKHR(vkstate->instance, vkstate->surface, NULL);
    vkDestroyInstance(vkstate->instance, NULL);
}