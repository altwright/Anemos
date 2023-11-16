#include "config.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "toml.h"

const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool loadUserConfig(const char* configFilePath, UserConfig *userConfig)
{
    assert(configFilePath);
    assert(userConfig);

    bool errorFree = true;

    FILE *fp = fopen(configFilePath, "r");
    if (!fp){
        perror("Failed to open user config file");
        return false;
    }

    char err[128] = {};
    toml_table_t *confToml = toml_parse_file(fp, err, sizeof(err));
    fclose(fp);
    if (!confToml){
        fprintf(stderr, "Failed to parse user config file: %s\n", err);
        return false;
    }

    const char* windowKey = "window";
    toml_table_t *windowTable = toml_table_in(confToml, windowKey);
    if (!windowTable){
        fprintf(stderr, "No %s table in %s\n", windowKey, configFilePath);
        errorFree = false;
    }

    toml_datum_t widthVal = toml_int_in(windowTable, "width");
    if (!widthVal.ok){
        fprintf(stderr, "Could not find width key for window conf\n");
        errorFree = false;
    }
    else {
        userConfig->window.width = widthVal.u.i;
    }

    toml_datum_t heightVal = toml_int_in(windowTable, "height");
    if (!heightVal.ok){
        fprintf(stderr, "Could not find height value in window conf\n");
        errorFree = false;
    }
    else {
        userConfig->window.height = heightVal.u.i;
    }

    toml_free(confToml);

    return errorFree;
}