#pragma once
#include "int.h"

#define WIDTH 800
#define HEIGHT 600

extern const char* TEXTURES_DIR;
extern const char* MODELS_DIR;
extern const char* SHADERS_DIR;

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

#define DEVICE_EXTENSIONS_COUNT 1
extern const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT];

#define MAX_FRAMES_IN_FLIGHT 2