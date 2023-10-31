#define GLFW_INCLUDE_VULKAN
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include "window.h"
#include "vkstate.h"
#include "vkinit.h"
#include "vkdestroy.h"

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
    vkstate.logicalDevice = createLogicalDevice(vkstate.physicalDevice, vkstate.surface);
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(vkstate.physicalDevice, vkstate.surface);
    vkGetDeviceQueue(vkstate.logicalDevice, queueFamilyIndices.graphicsQueue, 0, &vkstate.graphicsQueue);
    vkGetDeviceQueue(vkstate.logicalDevice, queueFamilyIndices.presentQueue, 0, &vkstate.presentQueue);
    vkstate.swapchain = createSwapchain(vkstate.logicalDevice, vkstate.physicalDevice, vkstate.surface, window.handle);
    vkstate.renderPass = createRenderPass(vkstate.logicalDevice, &vkstate.swapchain);
    vkstate.pipeline = createGraphicsPipeline(vkstate.logicalDevice, vkstate.renderPass, &vkstate.swapchain);
    vkstate.framebuffers = createFramebuffers(vkstate.logicalDevice, vkstate.renderPass, &vkstate.swapchain);

    while (!glfwWindowShouldClose(window.handle)) {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    destroyWindow(&window);
    destroyVkState(&vkstate);

    return 0;
}
