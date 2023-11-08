#define GLFW_INCLUDE_VULKAN
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include "window.h"
#include "vkstate.h"
#include "vkinit.h"
#include "vkdestroy.h"
#include "render.h"
#include "vertex.h"

#define WIDTH 640
#define HEIGHT 480

int main(int, char**){
    Window window = {};
    if (!createWindow(WIDTH, HEIGHT, "Anemos", &window)){
        printf("Failed to create GLFW window!");
        return EXIT_FAILURE;
    }

    VkState vkstate = initVkState(&window);

    copyVerticesToCoherentBuffer(vkstate.logicalDevice, vkstate.vertexBuffer, 0, vertices, VERTEX_COUNT);

    uint32_t currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle)) {
        vkWaitForFences(vkstate.logicalDevice, 1, &vkstate.frameStates[currentFrame].synchronisers.inFlight, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;//Will refer to a VkImage in our swapchain images array
        VkResult result = vkAcquireNextImageKHR(vkstate.logicalDevice, vkstate.swapchain.handle, UINT64_MAX, vkstate.frameStates[currentFrame].synchronisers.imageAvailable, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain(
                vkstate.logicalDevice, 
                vkstate.physicalDevice.handle, 
                vkstate.surface, 
                vkstate.renderPass, 
                window.handle,
                &vkstate.swapchain,
                &vkstate.framebuffers
            );
            continue;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            printf("Failed to Acquire Next Swapchain Image: %d\n", result);
            exit(EXIT_FAILURE);
        }

        vkResetFences(vkstate.logicalDevice, 1, &vkstate.frameStates[currentFrame].synchronisers.inFlight);

        vkResetCommandBuffer(vkstate.frameStates[currentFrame].commandBuffer, 0);

        recordDrawCommand(
            vkstate.frameStates[currentFrame].commandBuffer,
            vkstate.renderPass,
            vkstate.framebuffers.handles[imageIndex],
            vkstate.pipeline.handle,
            &vkstate.swapchain,
            vkstate.vertexBuffer,
            0,
            VERTEX_COUNT
        );

        submitDrawCommand(
            vkstate.graphicsQueue,
            vkstate.frameStates[currentFrame].commandBuffer,
            vkstate.frameStates[currentFrame].synchronisers.imageAvailable,
            vkstate.frameStates[currentFrame].synchronisers.renderFinished,
            vkstate.frameStates[currentFrame].synchronisers.inFlight
        );

        result = presentSwapchain(
            vkstate.presentQueue,
            vkstate.frameStates[currentFrame].synchronisers.renderFinished,
            vkstate.swapchain.handle,
            imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.resized){
            recreateSwapchain(
                vkstate.logicalDevice,
                vkstate.physicalDevice.handle,
                vkstate.surface,
                vkstate.renderPass,
                window.handle,
                &vkstate.swapchain,
                &vkstate.framebuffers
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

    vkDeviceWaitIdle(vkstate.logicalDevice);

    destroyWindow(&window);
    destroyVkState(&vkstate);

    return 0;
}
