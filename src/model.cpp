#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "cgltf.h"
#include "stb_image.h"

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

ModelInfo loadModelIntoStagingBuffer(const char *glbFilePath, u8 *mappedStagingBuffer)
{
    cgltf_options opts = {};
    cgltf_data *data = NULL;
    cgltf_result err = cgltf_parse_file(&opts, glbFilePath, &data);
    if (err)
    {
        fprintf(stderr, "Failed to load model %s: %d\n", glbFilePath, err);
        exit(EXIT_FAILURE);
    }

    cgltf_mesh* mesh = data->scene->nodes[0]->mesh;

    cgltf_primitive primitive = mesh->primitives[0];
    cgltf_accessor* positionsAccess = NULL;
    cgltf_accessor* texCoordAccess = NULL;
    for (size_t i = 0; i < primitive.attributes_count; i++)
    {
        cgltf_attribute attr = primitive.attributes[i];

        switch (attr.type)
        {
        case cgltf_attribute_type_position:
            positionsAccess = attr.data;
            break;
        case cgltf_attribute_type_texcoord:
            texCoordAccess = attr.data;
            break;
        default:
            break;
        }
    }

    if (!positionsAccess || !texCoordAccess || !primitive.indices || !primitive.material)
    {
        fprintf(stderr, "Could not load position, indices, tex coord or material attributes\n");
        exit(EXIT_FAILURE);
    }

    if (positionsAccess->component_type != cgltf_component_type_r_32f ||
        positionsAccess->type != cgltf_type_vec3)
    { 
        fprintf(stderr, "Positions are incorrect types\n");
        exit(EXIT_FAILURE);
    }

    if (primitive.indices->component_type != cgltf_component_type_r_16u ||
        primitive.indices->type != cgltf_type_scalar)
    {
        fprintf(stderr, "Indices are incorrect types\n");
        exit(EXIT_FAILURE);
    }

    if (texCoordAccess->component_type != cgltf_component_type_r_32f ||
        texCoordAccess->type != cgltf_type_vec2)
    {
        fprintf(stderr, "TexCoords are incorrect types\n");
        exit(EXIT_FAILURE);
    }

    u8 *binData = (unsigned char*)data->bin;
    u8 *positionsData = binData + positionsAccess->buffer_view->offset + positionsAccess->offset;
    u8 *indicesData = binData + primitive.indices->buffer_view->offset + primitive.indices->offset;
    u8 *texCoordData = binData + texCoordAccess->buffer_view->offset + texCoordAccess->offset;

    ModelInfo modelInfo = {.worldMatrix = GLM_MAT4_IDENTITY_INIT};
    modelInfo.verticesCount = positionsAccess->count;
    modelInfo.verticesDataSize = modelInfo.verticesCount * sizeof(vec3);
    modelInfo.indicesCount = primitive.indices->count;
    modelInfo.indicesDataSize = modelInfo.indicesCount * sizeof(u16);
    modelInfo.texCoordCount = texCoordAccess->count;
    modelInfo.texCoordDataSize = modelInfo.texCoordCount * sizeof(vec2);

    memcpy(mappedStagingBuffer, positionsData, modelInfo.verticesDataSize);
    mappedStagingBuffer += modelInfo.verticesDataSize;
    memcpy(mappedStagingBuffer, indicesData, modelInfo.indicesDataSize);
    mappedStagingBuffer += modelInfo.indicesDataSize;
    memcpy(mappedStagingBuffer, texCoordData, modelInfo.texCoordDataSize);
    mappedStagingBuffer += modelInfo.texCoordDataSize;

    if (!primitive.material->has_pbr_metallic_roughness)
    {
        fprintf(stderr, "Material is missing PBR component\n");
        exit(EXIT_FAILURE);
    }

    cgltf_image *image = primitive.material->pbr_metallic_roughness.base_color_texture.texture->image;
    u8 *imageData = binData + image->buffer_view->offset;

    stbi_uc *texData = stbi_load_from_memory(
        imageData, 
        image->buffer_view->size, 
        &modelInfo.texWidth, 
        &modelInfo.texHeight, 
        &modelInfo.texChannels, 
        STBI_rgb_alpha);

    if (!texData)
    {
        fprintf(stderr, "Invalid texture image\n");
        exit(EXIT_FAILURE);
    }

    memcpy(mappedStagingBuffer, texData, modelInfo.texWidth * modelInfo.texHeight * modelInfo.texChannels);

    stbi_image_free(texData);
    cgltf_free(data);

    return modelInfo;
}