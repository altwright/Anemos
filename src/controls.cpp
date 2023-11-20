#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

View cam_genViewMatrix(CameraControls *cam)
{
    View view = {};
    glm_lookat(cam->position, cam->focusPoint, cam->up, view.matrix);
    return view;
}

Projection cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea)
{
    Projection proj = {};
    glm_perspective(glm_rad(45.0f), renderArea.width / (float)renderArea.height, 0.1f, 10.0f, proj.matrix);
    proj.matrix[1][1] *= -1;//Originally designed for OpenGL, so must be inverted
    return proj;
}

void cam_handleKeyW(void *ctx, int action, int mods)
{
    if (action == GLFW_PRESS){
        CameraControls *cam = (CameraControls*)ctx;
        cam->position[0] -= 0.1f;
        cam->position[1] -= 0.1f;
        cam->position[2] -= 0.1f;
    }
}

void cam_handleKeyS(void *ctx, int action, int mods)
{
    if (action == GLFW_PRESS){
        CameraControls *cam = (CameraControls*)ctx;
        cam->position[0] += 0.1f;
        cam->position[1] += 0.1f;
        cam->position[2] += 0.1f;
    }
}

void cam_handleKeyD(void *ctx, int action, int mods)
{
    if (action == GLFW_PRESS){
        CameraControls *cam = (CameraControls*)ctx;
        cam->position[0] += 0.1f;
    }
}

void cam_handleKeyA(void *ctx, int action, int mods)
{
    if (action == GLFW_PRESS){
        CameraControls *cam = (CameraControls*)ctx;
        cam->position[0] -= 0.1f;
    }
}