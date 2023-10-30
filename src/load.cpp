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
    //Align unsigned char array to uint32_t
    size_t remainder = fileContents.len % sizeof(uint32_t);
    if (!remainder)
        fileContents.bytes = (unsigned char*)aligned_alloc(alignof(uint32_t), fileContents.len*sizeof(unsigned char));
    else
        fileContents.bytes = (unsigned char*)aligned_alloc(alignof(uint32_t), (fileContents.len + sizeof(uint32_t) - remainder)*sizeof(unsigned char));

    if (!fileContents.bytes){
        printf("Failed to allocate memory for %s\n", filepath);
        fileContents.bytes = NULL;
        fileContents.len = 0;
        return fileContents;
    }

    rewind(fp);
    if (fread(fileContents.bytes, sizeof(unsigned char), fileContents.len, fp) != fileContents.len){
        printf("Failed to read contents of %s\n", filepath);
        free(fileContents.bytes);
        fileContents.bytes = NULL;
        fileContents.len = 0;
        return fileContents;
    }

    fclose(fp);

    return fileContents;
}