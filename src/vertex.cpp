#include "vertex.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <assert.h>
#include "vkstate.h"

const Vertex vertices[VERTEX_COUNT] = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

VkVertexInputBindingDescription getVertexBindingDescription(){
    VkVertexInputBindingDescription bindingDesc = {0};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    //All of our per-vertex data is packed together in one array, 
    //so we’re only going to have one binding. The binding parameter 
    //specifies the index of the binding in the array of bindings.

    return bindingDesc;
}

VertexInputAttributes getVertexInputAttributes(){
    VertexInputAttributes attributes = {0};
    attributes.count = 2;
    attributes.descriptions = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription)*attributes.count);
    if (!attributes.descriptions){
        perror("Failed to malloc Vertex Input Attributes\n");
        exit(EXIT_FAILURE);
    }

    attributes.descriptions[0].binding = 0;
    attributes.descriptions[0].location = 0;
    attributes.descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes.descriptions[0].offset = offsetof(Vertex, pos);

    attributes.descriptions[1].binding = 0;
    attributes.descriptions[1].location = 1;
    attributes.descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes.descriptions[1].offset = offsetof(Vertex, colour);

    return attributes;
}

void copyVerticesToCoherentBuffer(VkDevice device, Buffer vertexBuffer, VkDeviceSize offset, const Vertex *vertices, size_t verticesCount)
{
    assert((sizeof(Vertex)*verticesCount) <= (vertexBuffer.size - offset));

    void *data = NULL;
    vkMapMemory(device, vertexBuffer.memory, offset, vertexBuffer.size, 0, &data);
    memcpy(data, vertices, sizeof(Vertex)*verticesCount);
    vkUnmapMemory(device, vertexBuffer.memory);
}
