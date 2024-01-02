#pragma once
#include <cglm/cglm.h>
#include "vkshader.h"

typedef struct CameraControls{
    vec3 position;
    versor globalOri;
    vec3 right;
    vec3 up;
    vec3 forward;
    float rotation_rad_s;

    double cursor_xpos, cursor_ypos;
    bool leftPressed;

    bool wPressed;
    s64 wPressedStart_ns;
    bool aPressed;
    s64 aPressedStart_ns;
    bool sPressed;
    s64 sPressedStart_ns;
    bool dPressed;
    s64 dPressedStart_ns;
} CameraControls;

CameraControls cam_createControls();
Matrix4 cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea);
Matrix4 cam_genViewMatrix(CameraControls *cam);

void cam_processInput(CameraControls *cam, GLFWwindow *window);
void cam_setInputHandler(CameraControls *cam, InputHandler *handler);
void cam_handleKeyW(void *ctx, int action, int mods);
void cam_handleKeyA(void *ctx, int action, int mods);
void cam_handleKeyS(void *ctx, int action, int mods);
void cam_handleKeyD(void *ctx, int action, int mods);
void cam_handleMouseScroll(void *ctx, double offset);
void cam_handleMouseClick(void *ctx, GLFWwindow *window, int button, int action, int mods);