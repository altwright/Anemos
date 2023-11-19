#pragma once

typedef struct CameraControls{
    float originDist;
} CameraControls;

void cam_handleKeyW(void *ctx, int action, int mods);
void cam_handleKeyS(void *ctx, int action, int mods);