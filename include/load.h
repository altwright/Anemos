#pragma once
#include <stdlib.h>
#include <vulkan/vulkan.h>

typedef struct FileContents{
    unsigned char *bytes;
    size_t len;
} FileContents;

FileContents readFileContents(const char *filepath);