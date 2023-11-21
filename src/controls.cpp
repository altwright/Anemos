#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "timing.h"

CameraControls cam_createControls()
{
    CameraControls camControls = {
        .position = {3.0f, 3.0f, 3.0f},
        .focusPoint = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 0.0f, 1.0f}};

    camControls.radPerSec = glm_rad(90.0f);

    camControls.dPressed = false;

    return camControls;
}

void cam_setInputHandler(CameraControls *cam, InputHandler *handler)
{
    handler->ctx = cam;
    handler->w = &cam_handleKeyW;
    handler->a = &cam_handleKeyA;
    handler->s = &cam_handleKeyS;
    handler->d = &cam_handleKeyD;
}

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

static s64 orbitCamLaterally(CameraControls *cam, s64 startTimeNs, float radPerSec)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 currentTimeNs = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiffNs = currentTimeNs - startTimeNs;
    vec3 rotationAxis = {};
    glm_vec3_add(cam->focusPoint, cam->up, rotationAxis);
    glm_vec3_rotate(cam->position, radPerSec*timeDiffNs/SEC_TO_NS(1), rotationAxis);

    return currentTimeNs;
}

void cam_processInput(CameraControls *cam)
{
    if (cam->aPressed){
        if (!cam->aPressedStartTimeNs){
            cam->aPressedStartTimeNs = getCurrentTimeNs();
        }
        else{
            cam->aPressedStartTimeNs = orbitCamLaterally(cam, cam->aPressedStartTimeNs, -1*cam->radPerSec);
        }
    }
    else 
        cam->aPressedStartTimeNs = 0;

    if (cam->dPressed){
        if (!cam->dPressedStartTimeNs){
            cam->dPressedStartTimeNs = getCurrentTimeNs();
        }
        else{
            cam->dPressedStartTimeNs = orbitCamLaterally(cam, cam->dPressedStartTimeNs, cam->radPerSec);
        }
    }
    else 
        cam->dPressedStartTimeNs = 0;
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
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->dPressed = true;
    }
    else{
        cam->dPressed = false;
    }
}

void cam_handleKeyA(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->aPressed = true;
    }
    else{
        cam->aPressed = false;
    }
}  