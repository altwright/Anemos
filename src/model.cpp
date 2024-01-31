#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "stb_image.h"

/*ModelInfo stageModelData(const char *glbFilePath, u8 *mappedStagingBuffer)
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
    if (!primitive.material->has_pbr_metallic_roughness)
    {
        fprintf(stderr, "Material is missing PBR component\n");
        exit(EXIT_FAILURE);
    }

    cgltf_image *image = primitive.material->pbr_metallic_roughness.base_color_texture.texture->image;
    u8 *imageData = binData + image->buffer_view->offset;

    int width, height = 0;
    stbi_uc *texData = stbi_load_from_memory(
        imageData, 
        image->buffer_view->size, 
        &width,
        &height,
        NULL,
        STBI_rgb_alpha
    );
    modelInfo.texWidth = width;
    modelInfo.texHeight = height;

    if (!texData)
    {
        fprintf(stderr, "Invalid texture image: %s\n", glbFilePath);
        exit(EXIT_FAILURE);
    }

    memcpy(mappedStagingBuffer, texData, modelInfo.texWidth * modelInfo.texHeight * STBI_rgb_alpha);

    stbi_image_free(texData);
    cgltf_free(data);

    return modelInfo;
}*/

ModelAttributeInfo stageModelVertexAttributes(cgltf_data *modelData, u8* stagingBuffer)
{
    cgltf_mesh* mesh = modelData->scene->nodes[0]->mesh;

    cgltf_primitive primitive = mesh->primitives[0];
    cgltf_accessor* verticesAccess = NULL;
    cgltf_accessor* texCoordAccess = NULL;
    for (size_t i = 0; i < primitive.attributes_count; i++)
    {
        cgltf_attribute attr = primitive.attributes[i];

        switch (attr.type)
        {
        case cgltf_attribute_type_position:
            verticesAccess = attr.data;
            break;
        case cgltf_attribute_type_texcoord:
            texCoordAccess = attr.data;
            break;
        default:
            break;
        }
    }

    if (!verticesAccess || !texCoordAccess)
    {
        fprintf(stderr, "Could not load position and tex coord attributes\n");
        abort();
    }

    if (verticesAccess->component_type != cgltf_component_type_r_32f ||
        verticesAccess->type != cgltf_type_vec3)
    { 
        fprintf(stderr, "Positions are incorrect types\n");
        abort();
    }

    if (texCoordAccess->component_type != cgltf_component_type_r_32f ||
        texCoordAccess->type != cgltf_type_vec2)
    {
        fprintf(stderr, "TexCoords are incorrect types\n");
        abort();
    }
    
    u8 *binData = (u8*)modelData->bin;
    vec3 *verticesData = (vec3*)(binData + verticesAccess->buffer_view->offset + verticesAccess->offset);
    vec2 *texCoordData = (vec2*)(binData + texCoordAccess->buffer_view->offset + texCoordAccess->offset);

    for(size_t i = 0; i < verticesAccess->count; i++)
    {
        memcpy(stagingBuffer, verticesData + i, sizeof(*verticesData));
        stagingBuffer += sizeof(*verticesData);

        memcpy(stagingBuffer, texCoordData + i, sizeof(*texCoordData));
        stagingBuffer += sizeof(*texCoordData);       
    }

    ModelAttributeInfo attrInfo = {.elementCount = verticesAccess->count};
    attrInfo.dataSize = attrInfo.elementCount * sizeof(*verticesData) + attrInfo.elementCount * sizeof(*texCoordData);

    return attrInfo;
}

cgltf_data* loadglTFData(const char *glbFilepath)
{
    cgltf_options opts = {};
    cgltf_data *data = NULL;
    cgltf_result err = cgltf_parse_file(&opts, glbFilepath, &data);
    if (err)
    {
        fprintf(stderr, "Failed to load model %s: %d\n", glbFilepath, err);
        exit(EXIT_FAILURE);
    }
    return data;
}

ModelAttributeInfo stageModelIndices(cgltf_data* modelData, u8* stagingBuffer)
{
    cgltf_mesh* mesh = modelData->scene->nodes[0]->mesh;
    cgltf_primitive primitive = mesh->primitives[0];

    if (!primitive.indices)
    {
        fprintf(stderr, "Could not load indices attributes\n");
        exit(EXIT_FAILURE);
    }

    if (primitive.indices->component_type != cgltf_component_type_r_16u ||
        primitive.indices->type != cgltf_type_scalar)
    {
        fprintf(stderr, "Indices are incorrect types\n");
        exit(EXIT_FAILURE);
    }

    u8 *binData = (u8*)modelData->bin;
    u16 *indicesData = (u16*)(binData + primitive.indices->buffer_view->offset + primitive.indices->offset);

    memcpy(stagingBuffer, indicesData, primitive.indices->count * sizeof(u16));

    ModelAttributeInfo attrInfo = {.elementCount = primitive.indices->count};
    attrInfo.dataSize = attrInfo.elementCount * sizeof(u16);

    return attrInfo;
}

TextureInfo stageModelTexture(cgltf_data *modelData, u8* stagingBuffer)
{
    cgltf_mesh* mesh = modelData->scene->nodes[0]->mesh;
    cgltf_primitive primitive = mesh->primitives[0];

    if (!primitive.material)
    {
        fprintf(stderr, "Could not load material attributes\n");
        exit(EXIT_FAILURE);
    }

    if (!primitive.material->has_pbr_metallic_roughness)
    {
        fprintf(stderr, "Material is missing PBR component\n");
        exit(EXIT_FAILURE);
    }

    u8 *binData = (u8*)modelData->bin;
    cgltf_image *image = primitive.material->pbr_metallic_roughness.base_color_texture.texture->image;
    u8 *imageData = binData + image->buffer_view->offset;

    int width, height = 0;
    stbi_uc *decodedTexture = stbi_load_from_memory(
        imageData, 
        image->buffer_view->size, 
        &width,
        &height,
        NULL,
        STBI_rgb_alpha);

    if (!decodedTexture)
    {
        fprintf(stderr, "Invalid texture image\n");
        exit(EXIT_FAILURE);
    }

    memcpy(stagingBuffer, decodedTexture, width * height * STBI_rgb_alpha);

    stbi_image_free(decodedTexture);

    TextureInfo texInfo = {.width = (u32)width, .height = (u32)height, .channels = STBI_rgb_alpha};

    return texInfo;
}