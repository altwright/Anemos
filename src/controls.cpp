#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "input.h"

void cam_handleKeyW(void *ctx, int action, int mods)
{
    CameraControlState *camState = (CameraControlState*)ctx;
    camState->count++;
    printf("Count: %d\n", camState->count);
}

void cam_handleKeyS(void *ctx, int action, int mods)
{
    CameraControlState *camState = (CameraControlState*)ctx;
    camState->count--;
    printf("Count: %d\n", camState->count);
}