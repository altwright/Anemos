#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "timing.h"

vec3 REF_UP = {0.0f, 1.0f, 0.0f};
vec3 REF_RIGHT = {1.0f, 0.0f, 0.0f};

CameraControls cam_createControls()
{
    CameraControls cam = {
        .position = {3.0f, 3.0f, 3.0f},
        .focus = GLM_VEC3_ZERO_INIT};

    glm_quat_forp(cam.position, cam.focus, REF_UP, cam.ori);

    cam.key_rotation_rad_s = glm_rad(90.0f);
    cam.mouse_rotation_rad = M_PI;

    return cam;
}

void cam_setInputHandler(CameraControls *cam, InputHandler *handler)
{
    handler->context = cam;
    handler->w = cam_handleKeyW;
    handler->a = cam_handleKeyA;
    handler->s = cam_handleKeyS;
    handler->d = cam_handleKeyD;
    handler->scroll = cam_handleMouseScroll;
    handler->click = cam_handleMouseClick;
}

Matrix4 cam_genViewMatrix(CameraControls *cam)
{
    Matrix4 view = {GLM_MAT4_IDENTITY_INIT};

    vec3 negPosition = {};
    glm_vec3_negate_to(cam->position, negPosition);
    mat4 translation = GLM_MAT4_IDENTITY_INIT;
    glm_translate_make(translation, negPosition);

    versor worldOri = {};//Move the world around the camera
    glm_quat_inv(cam->ori, worldOri);
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(worldOri, rotation);
    glm_mat4_mul_sse2(rotation, translation, view.matrix);

    return view;
}

Matrix4 cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea)
{
    Matrix4 proj = {};
    glm_perspective(glm_rad(45.0f), renderArea.width / (float)renderArea.height, 0.1f, 128.0f, proj.matrix);
    proj.matrix[1][1] *= -1;//Originally designed for OpenGL, so must be inverted
    return proj;
}

static s64 orbitCamLaterally(CameraControls *cam, s64 startTimeNs, float rad_s)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 current_ns = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiff_ns = current_ns - startTimeNs;

    float angle = timeDiff_ns * rad_s / SEC_TO_NS(1);

    vec3 relUp = {};
    glm_quat_rotatev(cam->ori, REF_UP, relUp);
    glm_vec3_rotate(cam->position, angle, relUp);
    glm_quat_forp(cam->position, cam->focus, relUp, cam->ori);

    return current_ns;
}

static s64 orbitCamLongitudinally(CameraControls *cam, s64 start_ns, float rad_s)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 current_ns = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiff_ns = current_ns - start_ns;

    float angle = timeDiff_ns * rad_s / SEC_TO_NS(1);

    vec3 relRight = {};
    glm_quat_rotatev(cam->ori, REF_RIGHT, relRight);
    glm_vec3_rotate(cam->position, angle, relRight);
    //printf("{%.2f, %.2f, %.2f}\n", relRight[0], relRight[1], relRight[2]);

    vec3 relUp = {};
    glm_quat_rotatev(cam->ori, REF_UP, relUp);
    glm_quat_forp(cam->position, cam->focus, relUp, cam->ori);

    return current_ns;
}

void cam_processInput(Window *window)
{
    CameraControls *cam = (CameraControls*)window->inputHandler.context;

    static bool leftPressedInit = true;
    if (cam->leftPressed)
    {
        static double cursor_xpos, cursor_ypos = 0.0f;
        if (leftPressedInit)
        {
            glfwGetCursorPos(window->handle, &cursor_xpos, &cursor_ypos);
            leftPressedInit = false;
        }
        else
        {
            double current_cursor_xpos, current_cursor_ypos = 0.0f;
            glfwGetCursorPos(window->handle, &current_cursor_xpos, &current_cursor_ypos);

            double xposDiff = current_cursor_xpos - cursor_xpos;
            double yposDiff = current_cursor_ypos - cursor_ypos;

            double xRotation = cam->mouse_rotation_rad*xposDiff/window->width;
            double yRotation = cam->mouse_rotation_rad*yposDiff/window->height;

            vec3 up = {};
            glm_quat_rotatev(cam->ori, REF_UP, up);
            glm_vec3_rotate(cam->position, -1*xRotation, up);
            //printf("{%.2f, %.2f, %.2f}\n", relUp[0], relUp[1], relUp[2]);

            vec3 right = {};
            glm_quat_rotatev(cam->ori, REF_RIGHT, right);
            glm_vec3_rotate(cam->position, -1*yRotation, right);
            //printf("{%.2f, %.2f, %.2f}\n", relRight[0], relRight[1], relRight[2]);

            glm_quat_forp(cam->position, cam->focus, up, cam->ori);

            cursor_xpos = current_cursor_xpos;
            cursor_ypos = current_cursor_ypos;
        }
    }
    else
    {
        leftPressedInit = true;
    }

    if (cam->wPressed)
    {
        if (!cam->wPressedStart_ns)
            cam->wPressedStart_ns = getCurrentTime_ns();
        else
            cam->wPressedStart_ns = orbitCamLongitudinally(cam, cam->wPressedStart_ns, -1*cam->key_rotation_rad_s);
    }
    else
    {
        cam->wPressedStart_ns = 0;
    }

    if (cam->aPressed)
    {
        if (!cam->aPressedStart_ns)
            cam->aPressedStart_ns = getCurrentTime_ns();
        else
            cam->aPressedStart_ns = orbitCamLaterally(cam, cam->aPressedStart_ns, -1*cam->key_rotation_rad_s);
    }
    else
    {
        cam->aPressedStart_ns = 0;
    }

    if (cam->sPressed){
        if (!cam->sPressedStart_ns)
            cam->sPressedStart_ns = getCurrentTime_ns();
        else
            cam->sPressedStart_ns = orbitCamLongitudinally(cam, cam->sPressedStart_ns, cam->key_rotation_rad_s);
    }
    else
        cam->sPressedStart_ns = 0;

    if (cam->dPressed){
        if (!cam->dPressedStart_ns)
            cam->dPressedStart_ns = getCurrentTime_ns();
        else
            cam->dPressedStart_ns = orbitCamLaterally(cam, cam->dPressedStart_ns, cam->key_rotation_rad_s);
    }
    else 
        cam->dPressedStart_ns = 0;

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

void cam_handleMouseScroll(void *ctx, double offset)
{
    CameraControls *cam = (CameraControls*)ctx;
    vec3 zoomOffset = {};
    glm_vec3_scale_as(cam->position, 0.5f, zoomOffset);

    if (offset > 0)//Scroll up and zoom in
    {
        glm_vec3_negate(zoomOffset);
        glm_vec3_add(zoomOffset, cam->position, cam->position);
    }
    else//Scroll down and zoom out
    {
        glm_vec3_add(zoomOffset, cam->position, cam->position);
    }
}

void cam_handleMouseClick(void *ctx, GLFWwindow *window, int button, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            cam->leftPressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE)
        {
            cam->leftPressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {

    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {

    }

}