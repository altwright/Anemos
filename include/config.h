#pragma once
#include "int.h"

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

#define DEVICE_EXTENSIONS_COUNT 1
extern const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT];

#define MAX_FRAMES_IN_FLIGHT 2
#define TEXTURES_DIR  "./textures/"
#define MODELS_DIR "./models/"
#define SHADERS_DIR "./shaders/"
#define CONFIG_FILE "./config.toml"
#define TITLE "ANEMOS"
#define VERSION VK_MAKE_VERSION(0, 1, 0)
#define VULKAN_VERSION VK_API_VERSION_1_3

typedef struct{
    struct {
        u32 width;
        u32 height;
    } window;
} UserConfig;

bool loadUserConfig(const char* configFilePath, UserConfig *userConfig);