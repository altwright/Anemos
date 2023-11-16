#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct Window{
    GLFWwindow *handle;
    int width;
    int height;
    bool resizing;
} Window;

bool createWindow(size_t width, size_t height, const char *title, Window *window);
void destroyWindow(Window *window);