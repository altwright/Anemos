#include "config.h"
#include <vulkan/vulkan.h>
#include <string.h>

const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char* TEXTURES_DIR = "./textures/";
const char* MODELS_DIR = "./models/";
const char* SHADERS_DIR = "./shaders/";