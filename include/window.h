#pragma once
#include <GLFW/glfw3.h>

typedef struct Window{
    GLFWwindow *handle;
    size_t width;
    size_t height;
} Window;

bool createWindow(size_t width, size_t height, const char *title, Window *window);
void destroyWindow(Window *window);