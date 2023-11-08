#include "vkdestroy.h"
#include <stdlib.h>

void destroySwapchainDetails(VkDevice device, SwapchainDetails *swapchainDetails){
    for (size_t i = 0; i < swapchainDetails->imagesCount; i++)
        vkDestroyImageView(device, swapchainDetails->imageViews[i], NULL);
    free(swapchainDetails->imageViews);
    free(swapchainDetails->images);
    vkDestroySwapchainKHR(device, swapchainDetails->handle, NULL);
}

void destroyFramebuffers(VkDevice device, Framebuffers *framebuffers){
    for(size_t i = 0; i < framebuffers->count; i++)
        vkDestroyFramebuffer(device, framebuffers->handles[i], NULL);
    free(framebuffers->handles);
}

void destroyVkState(VkState *vkstate){
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(vkstate->logicalDevice, vkstate->frameStates[i].synchronisers.imageAvailable, NULL);
        vkDestroySemaphore(vkstate->logicalDevice, vkstate->frameStates[i].synchronisers.renderFinished, NULL);
        vkDestroyFence(vkstate->logicalDevice, vkstate->frameStates[i].synchronisers.inFlight, NULL);
    }
    free(vkstate->frameStates);

    vkDestroyCommandPool(vkstate->logicalDevice, vkstate->graphicsCommandPool, NULL);
    vkDestroyCommandPool(vkstate->logicalDevice, vkstate->transferCommandPool, NULL);

    vkDestroyPipeline(vkstate->logicalDevice, vkstate->pipeline.handle, NULL);
    vkDestroyPipelineLayout(vkstate->logicalDevice, vkstate->pipeline.layout, NULL);

    destroyFramebuffers(vkstate->logicalDevice, &vkstate->framebuffers);
    
    vkDestroyRenderPass(vkstate->logicalDevice, vkstate->renderPass, NULL);

    destroySwapchainDetails(vkstate->logicalDevice, &vkstate->swapchain);

    vkDestroyBuffer(vkstate->logicalDevice, vkstate->vertexBuffer.handle, NULL);
    vkFreeMemory(vkstate->logicalDevice, vkstate->vertexBuffer.memory, NULL);
    vkDestroyBuffer(vkstate->logicalDevice, vkstate->indexBuffer.handle, NULL);
    vkFreeMemory(vkstate->logicalDevice, vkstate->indexBuffer.memory, NULL);

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