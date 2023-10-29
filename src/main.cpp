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

    VkState vkstate = {};
    vkstate.instance = createInstance(
        "Anemos", 
        VK_MAKE_VERSION(0, 1, 0),
        "Moebius",
        VK_MAKE_VERSION(0, 1, 0)
    );

    vkstate.physicalDevice = selectPhysicalDevice(vkstate.instance);

    while (!glfwWindowShouldClose(window.handle)) {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    destroyWindow(&window);
    destroyVkState(&vkstate);

    return 0;
}
