#define GLFW_INCLUDE_VULKAN
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include "window.h"

#define WIDTH 640
#define HEIGHT 480

int main(int, char**){
    GLFWwindow *window = createWindow(WIDTH, HEIGHT, "Anemos");
    if (!window){
        printf("Failed to create GLFW window!");
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    destroyWindow(window);

    return 0;
}
