#pragma once
#include <GLFW/glfw3.h>

typedef void (*HandleKey)(void *ctx, int action, int mods);

typedef struct InputHandler {
    void *ctx;
    HandleKey w;
    HandleKey s;
} InputHandler;

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
InputHandler createInputHandler();