#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "input.h"

void cam_handleKeyW(void *ctx, int action, int mods)
{
    CameraControls *camState = (CameraControls*)ctx;
    
    camState->originDist -= 0.1f;
}

void cam_handleKeyS(void *ctx, int action, int mods)
{
    CameraControls *camState = (CameraControls*)ctx;

    camState->originDist += 0.1f;
    
}