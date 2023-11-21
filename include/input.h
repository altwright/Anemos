#pragma once
#include <GLFW/glfw3.h>

typedef void (*HandleKey)(void *ctx, int action, int mods);

typedef struct InputHandler {
    void *ctx;
    HandleKey w;
    HandleKey a;
    HandleKey s;
    HandleKey d;
} InputHandler;

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resetInputHandler(InputHandler *handler);