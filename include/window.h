#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "input.h"

typedef struct Window{
    GLFWwindow *handle;
    int width;
    int height;
    InputHandler *inputHandler;
    bool resizing;
} Window;

bool initWindow(
    const char *title, 
    size_t width, 
    size_t height, 
    InputHandler *inputHandler, 
    Window *window);
void destroyWindow(Window *window);