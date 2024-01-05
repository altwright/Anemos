#include "vertex.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <assert.h>
#include "vkstate.h"

VkVertexInputBindingDescription getVertexBindingDescription()
{
    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(vec3) + sizeof(vec2); //Interleaved vertices + texCoords
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    //All of our per-vertex data is packed together in one array, 
    //so we’re only going to have one binding. The binding parameter 
    //specifies the index of the binding in the array of bindings.

    return bindingDesc;
}

VertexInputAttributeDescriptions getVertexInputAttributes()
{
    VertexInputAttributeDescriptions attributes = {};

    attributes.descs[0].binding = 0;
    attributes.descs[0].location = 0;
    attributes.descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes.descs[0].offset = offsetof(VertexAttributes, position);

    attributes.descs[1].binding = 0;
    attributes.descs[1].location = 1;
    attributes.descs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributes.descs[1].offset = offsetof(VertexAttributes, texCoord);

    return attributes;
}

/*
void copyVerticesToStagingBuffer(VkDevice device, Buffer vertexStagingBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount)
{
    assert((sizeof(Vertex)*verticesCount) <= (vertexStagingBuffer.size - offset));

    void *data = NULL;
    vkMapMemory(device, vertexStagingBuffer.memory, offset, vertexStagingBuffer.size, 0, &data);
    memcpy(data, vertices, sizeof(Vertex)*verticesCount);
    vkUnmapMemory(device, vertexStagingBuffer.memory);
}

void copyIndicesToStagingBuffer(VkDevice device, Buffer indexStagingBuffer, VkDeviceSize offset, const u16 *indices, size_t indicesCount)
{
    assert((sizeof(indices[0])*indicesCount) <= (indexStagingBuffer.size - offset));

    void *data = NULL;
    vkMapMemory(device, indexStagingBuffer.memory, offset, indexStagingBuffer.size, 0, &data);
    memcpy(data, indices, sizeof(indices[0])*indicesCount);
    vkUnmapMemory(device, indexStagingBuffer.memory);
}

*/