#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "timing.h"

CameraControls cam_createControls()
{
    CameraControls cam = {
        .worldPosition = {3.0f, 3.0f, 3.0f}};

    vec3 focus = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_quat_forp(cam.worldPosition, focus, up, cam.worldOrientation);
    glm_quat_inv(cam.worldOrientation, cam.worldOrientation);

    cam.radPerSec = glm_rad(90.0f);

    return cam;
}

void cam_setInputHandler(CameraControls *cam, InputHandler *handler)
{
    handler->ctx = cam;
    handler->w = &cam_handleKeyW;
    handler->a = &cam_handleKeyA;
    handler->s = &cam_handleKeyS;
    handler->d = &cam_handleKeyD;
}

Matrix4 cam_genViewMatrix(CameraControls *cam)
{
    Matrix4 view = {GLM_MAT4_IDENTITY_INIT};

    vec3 negatedPosition = {};
    glm_vec3_negate_to(cam->worldPosition, negatedPosition);
    mat4 translation = GLM_MAT4_IDENTITY_INIT;
    glm_translate_make(translation, negatedPosition);
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(cam->worldOrientation, rotation);
    glm_mat4_mul_sse2(rotation, translation, view.matrix);

    return view;
}

Matrix4 cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea)
{
    Matrix4 proj = {};
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

    return currentTimeNs;
}

static s64 orbitCamLongitudinally(CameraControls *cam, s64 startTimeNs, float radPerSec)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 currentTimeNs = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiffNs = currentTimeNs - startTimeNs;

    return currentTimeNs;
}

void cam_processInput(CameraControls *cam)
{
    if (cam->wPressed){
        if (!cam->wPressedStartTimeNs)
            cam->wPressedStartTimeNs = getCurrentTimeNs();
        else
            cam->wPressedStartTimeNs = orbitCamLongitudinally(cam, cam->wPressedStartTimeNs, -1*cam->radPerSec);
    }
    else
        cam->wPressedStartTimeNs = 0;

    if (cam->aPressed){
        if (!cam->aPressedStartTimeNs)
            cam->aPressedStartTimeNs = getCurrentTimeNs();
        else
            cam->aPressedStartTimeNs = orbitCamLaterally(cam, cam->aPressedStartTimeNs, -1*cam->radPerSec);
    }
    else 
        cam->aPressedStartTimeNs = 0;

    if (cam->sPressed){
        if (!cam->sPressedStartTimeNs)
            cam->sPressedStartTimeNs = getCurrentTimeNs();
        else
            cam->sPressedStartTimeNs = orbitCamLongitudinally(cam, cam->sPressedStartTimeNs, cam->radPerSec);
    }
    else
        cam->sPressedStartTimeNs = 0;

    if (cam->dPressed){
        if (!cam->dPressedStartTimeNs)
            cam->dPressedStartTimeNs = getCurrentTimeNs();
        else
            cam->dPressedStartTimeNs = orbitCamLaterally(cam, cam->dPressedStartTimeNs, cam->radPerSec);
    }
    else 
        cam->dPressedStartTimeNs = 0;
}

void cam_handleKeyW(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->wPressed = true;
    }
    else{
        cam->wPressed = false;
    }
}

void cam_handleKeyS(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->sPressed = true;
    }
    else{
        cam->sPressed = false;
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