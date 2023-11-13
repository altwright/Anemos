#define GLFW_INCLUDE_VULKAN
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include "window.h"
#include "vkstate.h"
#include "vkinit.h"
#include "vkdestroy.h"
#include "vkcommand.h"
#include "vertex.h"
#include "vkmemory.h"
#include "vkdescriptor.h"

#define WIDTH 640
#define HEIGHT 480

int main(int, char**){
    Window window = {};
    if (!createWindow(WIDTH, HEIGHT, "Anemos", &window)){
        printf("Failed to create GLFW window!");
        return EXIT_FAILURE;
    }

    VkState vk = initVkState(&window);

    copyDataToLocalBuffer(
        vk.device,
        &vk.physicalDevice,
        vk.transferCommandPool,
        vk.graphicsQueue,
        vk.vertexBuffer,
        0,
        vertices,
        VERTEX_COUNT,
        sizeof(vertices[0])
    );

    copyDataToLocalBuffer(
        vk.device,
        &vk.physicalDevice,
        vk.transferCommandPool,
        vk.graphicsQueue,
        vk.indexBuffer,
        0,
        indices,
        INDEX_COUNT,
        sizeof(indices[0])
    );

    uint32_t currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle)) {
        vkWaitForFences(vk.device, 1, &vk.frameControllers[currentFrame].synchronisers.inFlight, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;//Will refer to a VkImage in our swapchain images array
        VkResult result = vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, UINT64_MAX, vk.frameControllers[currentFrame].synchronisers.imageAvailable, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain(
                vk.device, 
                vk.physicalDevice.handle, 
                vk.surface, 
                vk.renderPass, 
                window.handle,
                &vk.swapchain,
                &vk.framebuffers
            );
            continue;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            printf("Failed to Acquire Next Swapchain Image: %d\n", result);
            exit(EXIT_FAILURE);
        }

        vkResetFences(vk.device, 1, &vk.frameControllers[currentFrame].synchronisers.inFlight);

        vkResetCommandBuffer(vk.frameControllers[currentFrame].commandBuffer, 0);

        updateUniformBuffer(vk.descriptors.sets[currentFrame].mappedBuffer, vk.swapchain.extent);

        recordDrawCommand(
            vk.frameControllers[currentFrame].commandBuffer,
            vk.renderPass,
            vk.framebuffers.handles[imageIndex],
            vk.graphicsPipeline,
            &vk.swapchain,
            vk.descriptors.sets[currentFrame].handle,
            vk.vertexBuffer,
            0,
            VERTEX_COUNT,
            vk.indexBuffer,
            0,
            INDEX_COUNT
        );

        submitDrawCommand(
            vk.graphicsQueue,
            vk.frameControllers[currentFrame].commandBuffer,
            vk.frameControllers[currentFrame].synchronisers.imageAvailable,
            vk.frameControllers[currentFrame].synchronisers.renderFinished,
            vk.frameControllers[currentFrame].synchronisers.inFlight
        );

        result = presentSwapchain(
            vk.presentQueue,
            vk.frameControllers[currentFrame].synchronisers.renderFinished,
            vk.swapchain.handle,
            imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.resized){
            recreateSwapchain(
                vk.device,
                vk.physicalDevice.handle,
                vk.surface,
                vk.renderPass,
                window.handle,
                &vk.swapchain,
                &vk.framebuffers
            );

            window.resized = false;
        }
        else if (result != VK_SUCCESS){
            printf("Failed to Present Swapchain Image\n");
            exit(EXIT_FAILURE);
        }
            
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    vkDeviceWaitIdle(vk.device);

    destroyWindow(&window);
    destroyVkState(&vk);

    return 0;
}
