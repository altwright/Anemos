#include "load.h"
#include <stdio.h>
#include <stdlib.h>

FileContents readFileContents(const char *filepath){
    FileContents fileContents{
        .bytes = NULL,
        .len = 0
    };

    FILE *fp = fopen(filepath, "rb");
    if (!fp){
        printf("Failed to open file %s\n", filepath);
        return fileContents;
    }

    if (fseek(fp, 0L, SEEK_END)){
        printf("Failed to seek the end of %s\n", filepath);
        return fileContents;
    }

    fileContents.len = ftell(fp);
    fileContents.bytes = (unsigned char*)malloc(sizeof(unsigned char)*fileContents.len);
    rewind(fp);

    if (fread(fileContents.bytes, sizeof(unsigned char), fileContents.len, fp) != fileContents.len){
        printf("Failed to read contents of %s\n", filepath);
        free(fileContents.bytes);
        fileContents.bytes = NULL;
        fileContents.len = 0;
        return fileContents;
    }

    return fileContents;
}