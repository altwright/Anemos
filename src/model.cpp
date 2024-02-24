#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "stb_image.h"

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

void getModelMatrix(mat4 outModelMatrix, cgltf_data *modelData)
{
    cgltf_node* node = modelData->scene->nodes[0];
    if (node->has_scale)
    {
        glm_scale(outModelMatrix, node->scale);
    }
}