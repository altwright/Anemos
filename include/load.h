#pragma once
#include <stdlib.h>
#include <vulkan/vulkan.h>

#define FILEPATH_SIZE 64

typedef struct{
    char str[FILEPATH_SIZE];
} FilePath;

typedef struct FileContents{
    unsigned char *bytes;
    size_t len;
} FileContents;

FileContents readFileContents(const char *filepath);
FilePath createFilePath(const char *dirPath, const char *fileName);