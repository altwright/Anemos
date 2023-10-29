#include "vkdestroy.h"

void destroyVkState(VkState *vkstate){
    vkDestroyInstance(vkstate->instance, NULL);
}