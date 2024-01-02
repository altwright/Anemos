#pragma once
#include <GLFW/glfw3.h>

typedef void (*HandleKey)(void *ctx, int action, int mods);
typedef void (*HandleMouseScroll)(void *ctx, double offset);
typedef void (*HandleMouseClick)(void *ctx, GLFWwindow *window, int button, int action, int mods);

typedef struct InputHandler {
    void *context;
    HandleKey w;
    HandleKey a;
    HandleKey s;
    HandleKey d;

    HandleMouseScroll scroll;
    HandleMouseClick click;
} InputHandler;

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resetInputHandler(InputHandler *handler);
void mouseScrollInputCallback(GLFWwindow* handle, double xoffset, double yoffset);
void mouseButtonInputCallback(GLFWwindow* handle, int button, int action, int mods);