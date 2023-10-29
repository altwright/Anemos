#include "window.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

GLFWwindow* createWindow(size_t width, size_t height, const char *title){
    /*
    if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)){
        printf("GLFW: Wayland supported\n");
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    }
    */

    if (!glfwInit()) {
        printf("Could not initialise GLFW!\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window){
        printf("Could not open window!\n");
        glfwTerminate();
        return NULL;
    }

    return window;
}

void destroyWindow(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
}