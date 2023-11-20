#include "input.h"
#include "window.h"

void keyInputCallback(GLFWwindow* handle, int key, int scancode, int action, int mods)
{
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    InputHandler *handler = window->inputHandler;
    void *context = handler->ctx;

    switch (key)
    {
        case GLFW_KEY_W:
            handler->w(context, action, mods);
            break;
        case GLFW_KEY_A:
            handler->a(context, action, mods);
            break;
        case GLFW_KEY_S:
            handler->s(context, action, mods);
            break;
        case GLFW_KEY_D:
            handler->d(context, action, mods);
            break;
        default:
            break;
    }
}

static void dropKey(void *ctx, int action, int mods){};

InputHandler createInputHandler()
{
    InputHandler handler = {};
    handler.w = dropKey;
    handler.a = dropKey;
    handler.s = dropKey;
    handler.d = dropKey;

    return handler;
}