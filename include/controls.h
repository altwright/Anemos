#pragma once
#include <cglm/cglm.h>
#include "vkshader.h"

typedef struct CameraControls{
    vec3 worldPosition;
    versor worldOrientation;

    float radPerSec;

    bool wPressed;
    s64 wPressedStartTimeNs;
    bool aPressed;
    s64 aPressedStartTimeNs;
    bool sPressed;
    s64 sPressedStartTimeNs;
    bool dPressed;
    s64 dPressedStartTimeNs;
} CameraControls;

CameraControls cam_createControls();
Matrix4 cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea);
Matrix4 cam_genViewMatrix(CameraControls *cam);
void cam_processInput(CameraControls *cam);
void cam_setInputHandler(CameraControls *cam, InputHandler *handler);
void cam_handleKeyW(void *ctx, int action, int mods);
void cam_handleKeyA(void *ctx, int action, int mods);
void cam_handleKeyS(void *ctx, int action, int mods);
void cam_handleKeyD(void *ctx, int action, int mods);