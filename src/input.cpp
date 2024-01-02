#include "input.h"
#include "window.h"

void mouseButtonInputCallback(GLFWwindow* handle, int button, int action, int mods)
{
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    InputHandler *handler = &window->inputHandler;
    void *context = handler->context;

    handler->click(context, handle, button, action, mods);
}

void mouseScrollInputCallback(GLFWwindow* handle, double xoffset, double yoffset)
{
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    InputHandler *handler = &window->inputHandler;
    void *context = handler->context;

    handler->scroll(context, yoffset);
}

void keyInputCallback(GLFWwindow* handle, int key, int scancode, int action, int mods)
{
    Window *window = (Window*)glfwGetWindowUserPointer(handle);
    InputHandler *handler = &window->inputHandler;
    void *context = handler->context;

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
static void dropMouseScroll(void *ctx, double offset){};
static void dropMouseClick(void *ctx, GLFWwindow *window, int button, int action, int mods){};

void resetInputHandler(InputHandler *handler)
{
    handler->context = NULL;
    handler->w = dropKey;
    handler->a = dropKey;
    handler->s = dropKey;
    handler->d = dropKey;
    handler->scroll = dropMouseScroll;
    handler->click = dropMouseClick;
}