#include "window.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

bool createWindow(size_t width, size_t height, const char *title, Window *window){
    /*
    if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)){
        printf("GLFW: Wayland supported\n");
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    }
    */

    if (!glfwInit()) {
        printf("Could not initialise GLFW!\n");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow* handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!handle){
        printf("Could not open window!\n");
        glfwTerminate();
        return false;
    }

    window->handle = handle;
    window->width = width;
    window->height = height;
    return true;
}

void destroyWindow(Window *window){
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}