#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "cgltf.h"

Model loadModel(const char *filePath)
{
    cgltf_options opts = {};
    cgltf_data *data = NULL;
    cgltf_result err = cgltf_parse_file(&opts, filePath, &data);
    if (err){
        fprintf(stderr, "Failed to load model %s: %d\n", filePath, err);
        exit(EXIT_FAILURE);
    }

    Model model = {};
    cgltf_attribute positionAttr = {};
    cgltf_attribute texCoordAttr = {};

    cgltf_primitive primitive = data->meshes[0].primitives[0];
    for (size_t i = 0; i < primitive.attributes_count; i++){
        if (primitive.attributes[i].type == cgltf_attribute_type_position){
            positionAttr = primitive.attributes[i];
            break;
        }

        if (primitive.attributes[i].type == cgltf_attribute_type_texcoord){
            texCoordAttr = primitive.attributes[i];
            break;
        }
    }

    model.verticesCount = positionAttr.data->count;
    model.vertices = (Vertex*)malloc(sizeof(Vertex)*model.verticesCount);
    if (!model.vertices){
        perror("Failed to malloc vertices for model");
        exit(EXIT_FAILURE);
    }

    cgltf_size verticesOffset = positionAttr.data->buffer_view->offset;
    float *verticesBuffer = (float*)((unsigned char*)data->bin + verticesOffset);
    for (size_t i = 0; i < model.verticesCount; i++){
        model.vertices[i] = {
            .position = {
                verticesBuffer[3*i], 
                verticesBuffer[3*i+1], 
                verticesBuffer[3*i+2], 
            }
        };
    }

    model.indicesCount = data->meshes[0].primitives[0].indices->count;
    model.indices = (u16*)malloc(sizeof(u16)*model.indicesCount);
    if (!model.indices){
        perror("Failed to malloc indices for model");
        exit(EXIT_FAILURE);
    }

    cgltf_accessor *indicesAccessor = data->meshes[0].primitives[0].indices;
    cgltf_size indicesOffset = indicesAccessor->buffer_view->offset;
    u16*indicesBuffer = (u16*)((unsigned char*)data->bin + indicesOffset);
    for (size_t i = 0; i < model.indicesCount; i++){
        model.indices[i] = indicesBuffer[i];
    }

    cgltf_free(data);
    
    glm_mat4_identity(model.worldMatrix);

    return model;
}

void freeModel(Model *model)
{
    free(model->vertices);
    free(model->indices);
}