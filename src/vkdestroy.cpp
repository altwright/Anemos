#include "vkdestroy.h"
#include <stdlib.h>
#include "config.h"

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

void destroyImage(VkDevice device, Image *image){
    vkDestroyImageView(device, image->view, NULL);
    vkDestroyImage(device, image->handle, NULL);
    vkFreeMemory(device, image->memory, NULL);
}

void _destroyVkState(VkState *vk){
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(vk->device, vk->frameControllers[i].synchronisers.imageAvailable, NULL);
        vkDestroySemaphore(vk->device, vk->frameControllers[i].synchronisers.renderFinished, NULL);
        vkDestroyFence(vk->device, vk->frameControllers[i].synchronisers.inFlight, NULL);
    }
    free(vk->frameControllers);

    vkDestroyCommandPool(vk->device, vk->graphicsCommandPool, NULL);
    vkDestroyCommandPool(vk->device, vk->transferCommandPool, NULL);

    vkDestroyPipeline(vk->device, vk->graphicsPipeline.handle, NULL);
    vkDestroyPipelineLayout(vk->device, vk->graphicsPipeline.layout, NULL);

    destroyFramebuffers(vk->device, &vk->framebuffers);
    
    vkDestroyRenderPass(vk->device, vk->renderPass, NULL);

    destroySwapchainDetails(vk->device, &vk->swapchain);

    for (size_t i = 0; i < vk->descriptors.setsCount; i++){
        vkUnmapMemory(vk->device, vk->descriptors.sets[i].buffer.memory);
        vkDestroyBuffer(vk->device, vk->descriptors.sets[i].buffer.handle, NULL);
        vkFreeMemory(vk->device, vk->descriptors.sets[i].buffer.memory, NULL);
    }
    free(vk->descriptors.sets);

    vkDestroyDescriptorPool(vk->device, vk->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(vk->device, vk->descriptors.layout, NULL);

    destroyImage(vk->device, &vk->sampleImage);
    destroyImage(vk->device, &vk->depthBuffer);

    vkDestroySampler(vk->device, vk->textureSampler, NULL);
    vkDestroyImageView(vk->device, vk->texture.view, NULL);
    vkDestroyImage(vk->device, vk->texture.handle, NULL);
    vkFreeMemory(vk->device, vk->texture.memory, NULL);

    vkDestroyBuffer(vk->device, vk->vertexBuffer.handle, NULL);
    vkFreeMemory(vk->device, vk->vertexBuffer.memory, NULL);
    vkDestroyBuffer(vk->device, vk->indexBuffer.handle, NULL);
    vkFreeMemory(vk->device, vk->indexBuffer.memory, NULL);

    vkDestroyDevice(vk->device, NULL);

    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);

    vkDestroyInstance(vk->instance, NULL);
}

