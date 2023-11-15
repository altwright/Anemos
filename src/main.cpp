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
#include "config.h"
#include "model.h"

int main(int, char**){
    Window window = {};
    if (!createWindow(WIDTH, HEIGHT, "Anemos", &window)){
        printf("Failed to create GLFW window!");
        return EXIT_FAILURE;
    }

    Model model = loadModel("./models/viking_room.obj");
    printf("Num Vertices: %ld\n", model.verticesCount);
    printf("Num Indices: %ld\n", model.indicesCount);

    VkState vk = initVkState(&window, model.verticesCount, model.indicesCount);

    copyDataToLocalBuffer(
        vk.device,
        &vk.physicalDevice,
        vk.transferCommandPool,
        vk.graphicsQueue,
        vk.vertexBuffer,
        0,
        model.vertices,
        model.verticesCount,
        sizeof(Vertex)
    );

    copyDataToLocalBuffer(
        vk.device,
        &vk.physicalDevice,
        vk.transferCommandPool,
        vk.graphicsQueue,
        vk.indexBuffer,
        0,
        model.indices,
        model.indicesCount,
        sizeof(u32)
    );

    uint32_t currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle)) {
        vkWaitForFences(vk.device, 1, &vk.frameControllers[currentFrame].synchronisers.inFlight, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;//Will refer to a VkImage in our swapchain images array
        VkResult result = vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, UINT64_MAX, vk.frameControllers[currentFrame].synchronisers.imageAvailable, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain(
                vk.device, 
                &vk.physicalDevice, 
                vk.surface, 
                vk.renderPass, 
                window.handle,
                &vk.swapchain,
                &vk.framebuffers,
                &vk.depthBuffer,
                &vk.sampleImage
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
            model.verticesCount,
            vk.indexBuffer,
            0,
            model.indicesCount
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
                &vk.physicalDevice,
                vk.surface,
                vk.renderPass,
                window.handle,
                &vk.swapchain,
                &vk.framebuffers,
                &vk.depthBuffer,
                &vk.sampleImage
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
