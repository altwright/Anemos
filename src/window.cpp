#include "window.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

static void framebufferResizeCallback(GLFWwindow *handle, int width, int height){
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    if (window){
        window->resizing = true;
        glfwGetWindowSize(handle, &window->width, &window->height);
    }
    else
        printf("Failed to retrieve GLFW User Pointer\n");
}

bool initWindow(
    const char *title, 
    size_t width, 
    size_t height, 
    InputHandler *inputHandler, 
    Window *window)
{
    if (!glfwInit()) {
        fprintf(stderr, "Could not initialise GLFW!\n");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow* handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!handle){
        fprintf(stderr, "Could not create GLFW Window!\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(handle, window);
    glfwSetFramebufferSizeCallback(handle, framebufferResizeCallback);
    glfwSetInputMode(handle, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetKeyCallback(handle, keyInputCallback);
    glfwSetScrollCallback(handle, mouseScrollInputCallback);

    window->handle = handle;
    window->width = width;
    window->height = height;
    window->inputHandler = inputHandler;
    window->resizing = false;

    return true;
}

void destroyWindow(Window *window){
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}