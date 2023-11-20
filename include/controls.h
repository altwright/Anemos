#pragma once
#include <cglm/cglm.h>
#include "vkshader.h"

typedef struct CameraControls{
    vec3 position;
    vec3 focusPoint;
    vec3 up;
} CameraControls;

Projection cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea);
View cam_genViewMatrix(CameraControls *cam);
void cam_handleKeyW(void *ctx, int action, int mods);
void cam_handleKeyA(void *ctx, int action, int mods);
void cam_handleKeyS(void *ctx, int action, int mods);
void cam_handleKeyD(void *ctx, int action, int mods);