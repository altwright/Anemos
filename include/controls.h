#pragma once

typedef struct CameraControlState{
    int count;
} CameraControlState;

void cam_handleKeyW(void *ctx, int action, int mods);
void cam_handleKeyS(void *ctx, int action, int mods);