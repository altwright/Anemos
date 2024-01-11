#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "cgltf.h"
#include "stb_image.h"

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

    u8 *binData = (u8*)data->bin;
    vec3 *positionsData = (vec3*)(binData + positionsAccess->buffer_view->offset + positionsAccess->offset);
    vec2 *texCoordData = (vec2*)(binData + texCoordAccess->buffer_view->offset + texCoordAccess->offset);
    u16 *indicesData = (u16*)(binData + primitive.indices->buffer_view->offset + primitive.indices->offset);

    ModelInfo modelInfo = {.worldMatrix = GLM_MAT4_IDENTITY_INIT};
    modelInfo.verticesCount = positionsAccess->count;
    modelInfo.verticesDataSize = modelInfo.verticesCount * sizeof(vec3);
    modelInfo.texCoordCount = texCoordAccess->count;
    modelInfo.texCoordDataSize = modelInfo.texCoordCount * sizeof(vec2);
    modelInfo.indicesCount = primitive.indices->count;
    modelInfo.indicesDataSize = modelInfo.indicesCount * sizeof(u16);

    if (modelInfo.verticesCount != modelInfo.texCoordCount)
    {
        fprintf(stderr, "TexCoord count does not match Vertices Count\n");
        exit(EXIT_FAILURE);
    }

    //Interleave vertex attributes
    for (size_t i = 0; i < modelInfo.verticesCount; i++)
    {
        memcpy(mappedStagingBuffer, positionsData + i, sizeof(vec3));
        mappedStagingBuffer += sizeof(vec3);

        memcpy(mappedStagingBuffer, texCoordData + i, sizeof(vec2));
        mappedStagingBuffer += sizeof(vec2);

        //printf("{%f, %f}\n", texCoordData[i][0], texCoordData[i][1]);
    }

    memcpy(mappedStagingBuffer, indicesData, modelInfo.indicesDataSize);
    mappedStagingBuffer += modelInfo.indicesDataSize;
/*
    printf("\n");
    u16 *indicies = (u16*)indicesData;
    for (int i = 0; i < modelInfo.indicesCount; i++){
        printf("%d\n", indicies[i]);
    }
*/
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
        NULL,
        STBI_rgb_alpha);

    if (!texData)
    {
        fprintf(stderr, "Invalid texture image: %s\n", glbFilePath);
        exit(EXIT_FAILURE);
    }

    memcpy(mappedStagingBuffer, texData, modelInfo.texWidth * modelInfo.texHeight * STBI_rgb_alpha);

    stbi_image_free(texData);
    cgltf_free(data);

    return modelInfo;
}