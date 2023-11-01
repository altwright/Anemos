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

#define WIDTH 640
#define HEIGHT 480

int main(int, char**){
    Window window = {};
    if (!createWindow(WIDTH, HEIGHT, "Anemos", &window)){
        printf("Failed to create GLFW window!");
        return EXIT_FAILURE;
    }

    VkState vkstate{};
    vkstate.instance = createInstance("Anemos", VK_MAKE_VERSION(0, 1, 0), "Moebius", VK_MAKE_VERSION(0, 1, 0));
    vkstate.surface = createSurface(vkstate.instance, window.handle);
    vkstate.physicalDevice = selectPhysicalDevice(vkstate.instance, vkstate.surface);
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(vkstate.physicalDevice, vkstate.surface);
    vkstate.logicalDevice = createLogicalDevice(vkstate.physicalDevice, queueFamilyIndices);
    vkGetDeviceQueue(vkstate.logicalDevice, queueFamilyIndices.graphicsQueue, 0, &vkstate.graphicsQueue);
    vkGetDeviceQueue(vkstate.logicalDevice, queueFamilyIndices.presentQueue, 0, &vkstate.presentQueue);
    vkstate.swapchain = createSwapchain(vkstate.logicalDevice, vkstate.physicalDevice, vkstate.surface, window.handle);
    vkstate.renderPass = createRenderPass(vkstate.logicalDevice, &vkstate.swapchain);
    vkstate.pipeline = createGraphicsPipeline(vkstate.logicalDevice, vkstate.renderPass, &vkstate.swapchain);
    vkstate.framebuffers = createFramebuffers(vkstate.logicalDevice, vkstate.renderPass, &vkstate.swapchain);
    vkstate.commandBuffers = createCommandBuffer(vkstate.logicalDevice, queueFamilyIndices);
    vkstate.synchronisers = createSynchronisers(vkstate.logicalDevice);

    while (!glfwWindowShouldClose(window.handle)) {
        vkWaitForFences(vkstate.logicalDevice, 1, &vkstate.synchronisers.inFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(vkstate.logicalDevice, 1, &vkstate.synchronisers.inFlight);

        uint32_t imageIndex;//Will refer to a VkImage in our swapchain images array
        vkAcquireNextImageKHR(vkstate.logicalDevice, vkstate.swapchain.handle, UINT64_MAX, vkstate.synchronisers.imageAvailable, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(vkstate.commandBuffers.handle, 0);
        recordDrawCommand(
            vkstate.commandBuffers.handle,
            vkstate.renderPass,
            vkstate.framebuffers.handles[imageIndex],
            vkstate.pipeline.handle,
            &vkstate.swapchain
        );
        submitDrawCommand(
            vkstate.graphicsQueue,
            vkstate.commandBuffers.handle,
            vkstate.synchronisers.imageAvailable,
            vkstate.synchronisers.renderFinished,
            vkstate.synchronisers.inFlight
        );
        presentSwapchain(
            vkstate.presentQueue,
            vkstate.synchronisers.renderFinished,
            vkstate.swapchain.handle,
            imageIndex
        );
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    vkDeviceWaitIdle(vkstate.logicalDevice);

    destroyWindow(&window);
    destroyVkState(&vkstate);

    return 0;
}
