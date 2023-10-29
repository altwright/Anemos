#define GLFW_INCLUDE_VULKAN
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480

int main(int, char**){
    if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)){
        printf("GLFW: Wayland supported\n");
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    }

    if (!glfwInit()) {
        printf("Could not initialise GLFW!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Anemos", NULL, NULL);
    if (!window){
        printf("Could not open window!\n");
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
