#include "vkdestroy.h"

void destroyVkState(VkState *vkstate){
    vkDestroyDevice(vkstate->logicalDevice, NULL);
    vkDestroyInstance(vkstate->instance, NULL);
}