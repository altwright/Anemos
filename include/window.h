#pragma once
#include <GLFW/glfw3.h>

typedef struct Window{
    GLFWwindow *handle;
    int width;
    int height;
    bool resized;
} Window;

bool createWindow(size_t width, size_t height, const char *title, Window *window);
void destroyWindow(Window *window);