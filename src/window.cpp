#include "window.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

static void framebufferResizeCallback(GLFWwindow *handle, int width, int height){
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    if (window){
        window->resized = true;
        glfwGetWindowSize(handle, &window->width, &window->height);
    }
    else
        printf("Failed to retrieve GLFW User Pointer\n");
}

bool createWindow(size_t width, size_t height, const char *title, Window *window){
    if (!glfwInit()) {
        printf("Could not initialise GLFW!\n");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow* handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!handle){
        printf("Could not open window!\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(handle, window);
    glfwSetFramebufferSizeCallback(handle, framebufferResizeCallback);

    window->handle = handle;
    window->width = width;
    window->height = height;
    window->resized = false;

    return true;
}

void destroyWindow(Window *window){
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}