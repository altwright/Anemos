#include "model.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "tinyobj_loader_c.h"

static void objFileReader(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len)
{
    *data = NULL;
    *len = 0;

    if (!is_mtl){
        int fd = open(filename, O_RDONLY);
        if (fd == -1){
            perror("Failed to open OBJ file ");
            fprintf(stderr, "%s\n", filename);
            return;
        }

        struct stat fdInfo = {};

        if (fstat(fd, &fdInfo) == -1){
            perror("Failed to use fstat()\n");
            return;
        }

        *data = (char*)mmap(NULL, fdInfo.st_size, PROT_READ, MAP_SHARED, fd, 0);

        if (*data == MAP_FAILED){
            perror("Memory mapping failed\n");
            *data = NULL;
            return;
        }

        if (close(fd) == -1){
            perror("Failed to close file descriptor to OBJ file\n");
            *data = NULL;
            return;
        }

        *len = fdInfo.st_size;
    }
}

Model loadModel(const char *filePath){
    tinyobj_attrib_t attrib = {};
    tinyobj_shape_t *shapes = NULL;
    size_t shapesCount = 0;
    tinyobj_material_t *materials = NULL;
    size_t materialsCount = 0;

    if (tinyobj_parse_obj(
        &attrib, 
        &shapes, &shapesCount, 
        &materials, &materialsCount, 
        filePath, objFileReader, NULL, 
        TINYOBJ_FLAG_TRIANGULATE))
    {
        fprintf(stderr, "Failed to parse and load OBJ file %s\n", filePath);
        exit(EXIT_FAILURE);
    }

    Model model = {};
    model.verticesCount = attrib.num_faces;
    model.vertices = (Vertex*)malloc(sizeof(Vertex)*model.verticesCount);
    if (!model.vertices){
        perror("Failed to malloc Model Vertices");
        exit(EXIT_FAILURE);
    }
    model.indicesCount = attrib.num_faces;
    model.indices = (u32*)malloc(sizeof(u32)*model.indicesCount);
    if (!model.indices){
        perror("Failed to malloc Model Indices");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < attrib.num_faces; i++){
        model.vertices[i] = {
            .pos = {
                attrib.vertices[3*attrib.faces[i].v_idx],
                attrib.vertices[3*attrib.faces[i].v_idx+1],
                attrib.vertices[3*attrib.faces[i].v_idx+2]
            },
            .colour = {1.0f, 1.0f, 1.0f},
            .texCoord = {
                attrib.texcoords[2*attrib.faces[i].vt_idx],
                1.0f - attrib.texcoords[2*attrib.faces[i].vt_idx+1]
            }
        };

        model.indices[i] = i;
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, shapesCount);
    tinyobj_materials_free(materials, materialsCount);

    return model;
}