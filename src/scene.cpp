#include "scene.h"
#include <string.h>
#include "vkmemory.h"
#include "stb_image.h"
#include "vkcommand.h"
#include "cgltf.h"

SceneInfo loadSceneToDevice(
    const char *surfaceFilepath, 
    const char *characterFilepath, 
    Buffer stagingBuffer, Buffer deviceBuffer,
    VkDevice device,
    VmaAllocator allocator,
    VkCommandPool cmdPool,
    VkQueue queue)
{
    u8* mappedSB = (u8*)stagingBuffer.info.pMappedData;
    size_t sbOffset = 0;

    size_t vtxBufOffset = sbOffset;

    cgltf_data* surfaceData = loadglTFData(surfaceFilepath);
    ModelAttributeInfo surfaceVtxAttrInfo = stageModelVertexAttributes(surfaceData, mappedSB + sbOffset);
    sbOffset += surfaceVtxAttrInfo.dataSize;

    cgltf_data* characterData = loadglTFData(characterFilepath);
    ModelAttributeInfo characterVtxAttrInfo = stageModelVertexAttributes(characterData, mappedSB + sbOffset);
    sbOffset += characterVtxAttrInfo.dataSize;

    size_t idxBufOffset = sbOffset;

    ModelAttributeInfo surfaceIndicesInfo = stageModelIndices(surfaceData, mappedSB + sbOffset);
    sbOffset += surfaceIndicesInfo.dataSize;

    ModelAttributeInfo characterIndicesInfo = stageModelIndices(characterData, mappedSB + sbOffset);
    sbOffset += characterIndicesInfo.dataSize;

    size_t drawCmdsOffset = sbOffset;

    VkDrawIndexedIndirectCommand surfaceIndirectDrawCmd = {};
    surfaceIndirectDrawCmd.firstIndex = 0;
    surfaceIndirectDrawCmd.indexCount = surfaceIndicesInfo.elementCount;
    surfaceIndirectDrawCmd.vertexOffset = 0;
    surfaceIndirectDrawCmd.firstInstance = 0;
    surfaceIndirectDrawCmd.instanceCount = 1;

    VkDrawIndexedIndirectCommand characterIndirectDrawCmd = {};
    characterIndirectDrawCmd.firstIndex = surfaceIndicesInfo.elementCount;
    characterIndirectDrawCmd.indexCount = characterIndicesInfo.elementCount;
    characterIndirectDrawCmd.vertexOffset = surfaceVtxAttrInfo.elementCount;
    characterIndirectDrawCmd.firstInstance = 1;
    characterIndirectDrawCmd.instanceCount = 1;

    VkDrawIndexedIndirectCommand indirectDrawCmds[] = {surfaceIndirectDrawCmd, characterIndirectDrawCmd};
    memcpy(mappedSB + sbOffset, indirectDrawCmds, sizeof(indirectDrawCmds));
    sbOffset += sizeof(indirectDrawCmds);

    size_t texBufOffset = sbOffset;

    TextureInfo surfaceTexInfo = stageModelTexture(surfaceData, mappedSB + sbOffset);
    DeviceImage surfaceTex = createDeviceTexture(device, allocator, surfaceTexInfo.width, surfaceTexInfo.height);
    size_t surfaceTexSize = surfaceTexInfo.width * surfaceTexInfo.height * surfaceTexInfo.channels;
    sbOffset += surfaceTexSize;

    TextureInfo characterTexInfo = stageModelTexture(characterData, mappedSB + sbOffset);
    DeviceImage characterTex = createDeviceTexture(device, allocator, characterTexInfo.width, characterTexInfo.height);

    VkCommandBuffer cmdBuffer = beginSingleTimeCommandBuffer(device, cmdPool);

    VkBufferCopy modelDetailsCopyRegion = {//Include Indirect Draw Commands
        .srcOffset = 0,
        .dstOffset = 0,
        .size = texBufOffset
    };

    vkCmdCopyBuffer(cmdBuffer, stagingBuffer.handle, deviceBuffer.handle, 1, &modelDetailsCopyRegion);

    // If there is a semaphore signal + wait between this being submitted and
    // the vertex buffer being used, then skip this pipeline barrier.
    /*VkMemoryBarrier2 meshTransferBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
    meshTransferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    meshTransferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    meshTransferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    meshTransferBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;*/

    VkImageMemoryBarrier2 surfaceTexTransitionBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    surfaceTexTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    surfaceTexTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    surfaceTexTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    surfaceTexTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    surfaceTexTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    surfaceTexTransitionBarrier.subresourceRange.baseMipLevel = 0;
    surfaceTexTransitionBarrier.subresourceRange.levelCount = 1;
    surfaceTexTransitionBarrier.subresourceRange.baseArrayLayer = 0;
    surfaceTexTransitionBarrier.subresourceRange.layerCount = 1;
    surfaceTexTransitionBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE_KHR;
    surfaceTexTransitionBarrier.srcAccessMask = VK_ACCESS_2_NONE;
    surfaceTexTransitionBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    surfaceTexTransitionBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    surfaceTexTransitionBarrier.image = surfaceTex.handle;

    VkImageMemoryBarrier2 characterTexTransitionBarrier = surfaceTexTransitionBarrier;
    characterTexTransitionBarrier.image = characterTex.handle;

    VkImageMemoryBarrier2 texTransitionsBarriers[] = {surfaceTexTransitionBarrier, characterTexTransitionBarrier};

    VkDependencyInfo texTransitionInfo = {};
    texTransitionInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    texTransitionInfo.imageMemoryBarrierCount = NUM_ELEMENTS(texTransitionsBarriers);
    texTransitionInfo.pImageMemoryBarriers = texTransitionsBarriers;

    vkCmdPipelineBarrier2(cmdBuffer, &texTransitionInfo);

    VkBufferImageCopy surfaceTexCopy = {};
    surfaceTexCopy.bufferOffset = texBufOffset;
    surfaceTexCopy.bufferRowLength = 0;
    surfaceTexCopy.bufferImageHeight = 0;
    surfaceTexCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    surfaceTexCopy.imageSubresource.mipLevel = 0;
    surfaceTexCopy.imageSubresource.baseArrayLayer = 0;
    surfaceTexCopy.imageSubresource.layerCount = 1;
    surfaceTexCopy.imageOffset = {0, 0, 0};
    surfaceTexCopy.imageExtent = {surfaceTexInfo.width, surfaceTexInfo.height, 1};

    vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer.handle,
        surfaceTex.handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &surfaceTexCopy
    );

    VkBufferImageCopy characterTexCopy = {};
    characterTexCopy.bufferOffset = texBufOffset + surfaceTexSize;
    characterTexCopy.bufferRowLength = 0;
    characterTexCopy.bufferImageHeight = 0;
    characterTexCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    characterTexCopy.imageSubresource.mipLevel = 0;
    characterTexCopy.imageSubresource.baseArrayLayer = 0;
    characterTexCopy.imageSubresource.layerCount = 1;
    characterTexCopy.imageOffset = {0, 0, 0};
    characterTexCopy.imageExtent = {characterTexInfo.width, characterTexInfo.height, 1};

    vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer.handle,
        characterTex.handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &characterTexCopy
    );

    for(size_t i = 0; i < NUM_ELEMENTS(texTransitionsBarriers); i++)
    {
        texTransitionsBarriers[i].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        texTransitionsBarriers[i].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        texTransitionsBarriers[i].srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        texTransitionsBarriers[i].srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        texTransitionsBarriers[i].dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        texTransitionsBarriers[i].dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    }

    vkCmdPipelineBarrier2(cmdBuffer, &texTransitionInfo);

    submitSingleTimeCommandBuffer(device, cmdPool, cmdBuffer, queue);

    SceneInfo sceneInfo = {};
    sceneInfo.vtxBufOffset = vtxBufOffset;
    sceneInfo.idxBufOffset = idxBufOffset;
    sceneInfo.drawCmdsOffset = drawCmdsOffset;
    sceneInfo.drawCmdsCount = NUM_ELEMENTS(indirectDrawCmds);
    sceneInfo.surfaceModelInfo = {.modelMatrix = GLM_MAT4_IDENTITY_INIT, .tex = surfaceTex};
    getModelMatrix(sceneInfo.surfaceModelInfo.modelMatrix, surfaceData);
    sceneInfo.characterModelInfo = {.modelMatrix = GLM_MAT4_IDENTITY_INIT, .tex = characterTex};
    getModelMatrix(sceneInfo.characterModelInfo.modelMatrix, characterData);

    sceneInfo.surfaceVoxels = calcSurfaceVoxels(surfaceData, sceneInfo.surfaceModelInfo.modelMatrix);

    cgltf_free(surfaceData);
    cgltf_free(characterData);

    return sceneInfo;
}

void freeSceneInfo(SceneInfo *info)
{
    free(info->surfaceVoxels.data);
}

Voxels calcSurfaceVoxels(cgltf_data *surfaceData, mat4 modelMatrix)
{
    cgltf_mesh* mesh = surfaceData->scene->nodes[0]->mesh;
    cgltf_primitive primitive = mesh->primitives[0];
    cgltf_accessor* verticesAccess = NULL;

    for (size_t i = 0; i < primitive.attributes_count; i++)
    {
        cgltf_attribute attr = primitive.attributes[i];

        switch (attr.type)
        {
        case cgltf_attribute_type_position:
            verticesAccess = attr.data;
            break;
        default:
            break;
        }
    }

    u8 *binData = (u8*)surfaceData->bin;
    vec3 *verticesData = (vec3*)(binData + verticesAccess->buffer_view->offset + verticesAccess->offset);
    u16 *indicesData = (u16*)(binData + primitive.indices->buffer_view->offset + primitive.indices->offset);

    Voxels voxels = {
        .cols = 2,
        .rows = 2,
        .depth = 2,
        .origin = {-8.0f, 8.0f, -8.0f},//Nearest upper left corner
        .voxWidth = 8.0f,
        .voxHeight = 8.0f,
        .voxLength = 8.0f
    };

    u64 numVoxels = voxels.cols*voxels.rows*voxels.depth;
    u64 storedIndicesMaxDataSize = 3 * 3 * primitive.indices->count * sizeof(u16);//A triangle can cross into three voxels max, and each triangle is three indices

    voxels.dataSize = numVoxels * sizeof(u16)//For the voxel indices into the data array 
        + storedIndicesMaxDataSize
        + verticesAccess->count * sizeof(vec3)//Store a copy of the transformed vertices, since we can't read them from the staging buffer
        + numVoxels * sizeof(u16);//Temporary counter array
    
    voxels.data = (u8*)malloc(sizeof(u8)*voxels.dataSize);
    if (!voxels.data)
    {
        fprintf(stderr, "Failed to allocate Surface Test Voxel Data");
        abort();
    }

    memset(voxels.data, 0, voxels.dataSize);

    //Store the copy of the vertex buffer after the index array and stored indices
    u64 transformedVerticesIdx = sizeof(u16)*numVoxels + storedIndicesMaxDataSize;
    vec3 *transformedVertices = (vec3*)(voxels.data + transformedVerticesIdx);

    //Pre-transform all vertices into model-space
    for(u64 i = 0; i < verticesAccess->count; i++)
    {
        glm_mat4_mulv3(modelMatrix, verticesData[i], 0.0f, transformedVertices[i]);
    }

    voxels.transformedVerticesIdx = transformedVerticesIdx;

    u16 *voxelIndicesCounts = (u16*)voxels.data;

    //Count how many indices fall into each voxel
    for(u64 i = 0; i < primitive.indices->count; i += 3)
    {
        //Used to avoid storing an indice in the same voxel
        int prevVoxelIdxs[3] = {-1, -1, -1};

        for(u64 vtx = 0; vtx < 3; vtx++)
        {
            float x = verticesData[indicesData[i + vtx]][0];
            float y = verticesData[indicesData[i + vtx]][1];
            float z = verticesData[indicesData[i + vtx]][2];

            int colIdx = -1, rowIdx = -1, depthIdx = -1;

            if (x >= voxels.origin[0] && x < (voxels.origin[0] + voxels.voxWidth*voxels.cols))
            {
                colIdx = (x - voxels.origin[0])/(voxels.voxWidth);
            }

            if (y > (voxels.origin[1] - voxels.voxHeight*voxels.rows) && y <= voxels.origin[1])
            {
                rowIdx = (voxels.origin[1] - y)/(voxels.voxHeight);
            }

            if (z >= voxels.origin[2] && z < (voxels.origin[2] + voxels.voxLength*voxels.depth))
            {
                depthIdx = (z - voxels.origin[2])/(voxels.voxLength);
            }

            if (colIdx >= 0 && rowIdx >= 0 && depthIdx >= 0)
            {
                int voxelIdx = colIdx*(voxels.rows*voxels.depth) + rowIdx*(voxels.depth) + depthIdx;
                bool uniqueVoxel = true;

                for (size_t j = 0; j < vtx; j++)
                {
                    if (voxelIdx == prevVoxelIdxs[j])
                        uniqueVoxel = false;
                }

                if (uniqueVoxel)
                {
                    voxelIndicesCounts[voxelIdx] += 3;
                    prevVoxelIdxs[vtx] = voxelIdx;
                }
            }
        }
    }

    u32 voxelIndicesIdx = 0;
    for (u64 i = 0; i < numVoxels; i++)
    {
        u16 count = voxelIndicesCounts[i];
        voxelIndicesCounts[i] = voxelIndicesIdx;//They now become absolute indices instead of relative
        voxelIndicesIdx += count;
    }

    voxels.storedIndicesCount = voxelIndicesIdx;

    u16 *voxelIndicesCounter = (u16*)(voxels.data + transformedVerticesIdx + sizeof(vec3)*verticesAccess->count);//Fit temporary counter on the end of the data buffer
    u16 *voxelIndices = (u16*)(voxels.data + sizeof(u16)*numVoxels);//Skip past the index array

    for (u64 i = 0; i < primitive.indices->count; i += 3)
    {
        int prevVoxelIdxs[3] = {-1, -1, -1};

        for (u64 vtx = 0; vtx < 3; vtx++)
        {
            float x = verticesData[indicesData[i + vtx]][0];
            float y = verticesData[indicesData[i + vtx]][1];
            float z = verticesData[indicesData[i + vtx]][2];

            int colIdx = -1, rowIdx = -1, depthIdx = -1;

            if (x >= voxels.origin[0] && x < (voxels.origin[0] + voxels.voxWidth*voxels.cols))
            {
                colIdx = (x - voxels.origin[0])/(voxels.voxWidth);
            }

            if (y > (voxels.origin[1] - voxels.voxHeight*voxels.rows) && y <= voxels.origin[1])
            {
                rowIdx = (voxels.origin[1] - y)/(voxels.voxHeight);
            }

            if (z >= voxels.origin[2] && z < (voxels.origin[2] + voxels.voxLength*voxels.depth))
            {
                depthIdx = (z - voxels.origin[2])/(voxels.voxLength);
            }

            if (colIdx >= 0 && rowIdx >= 0 && depthIdx >= 0)//If it lies within the voxels
            {
                int voxelIdx = colIdx*(voxels.rows*voxels.depth) + rowIdx*(voxels.depth) + depthIdx;
                bool uniqueVoxel = true;

                for (size_t j = 0; j < vtx; j++)
                {
                    if (voxelIdx == prevVoxelIdxs[j])
                        uniqueVoxel = false;
                }

                if (uniqueVoxel)
                {
                    size_t indicesIdx = voxelIndicesCounts[voxelIdx];
                    voxelIndices[indicesIdx + voxelIndicesCounter[voxelIdx]] = indicesData[i];
                    voxelIndices[indicesIdx + voxelIndicesCounter[voxelIdx] + 1] = indicesData[i + 1];
                    voxelIndices[indicesIdx + voxelIndicesCounter[voxelIdx] + 2] = indicesData[i + 2];
                    voxelIndicesCounter[voxelIdx] += 3;
                }
            }
        }
    }

// #ifndef NDEBUG
//     printf("\nStored Indices: %d\n", voxels.storedIndicesCount);
//     printf("\tVoxel: Indices Index\n");
//     for (int i = 0; i < numVoxels; i++)
//     {
//         printf("\t%d: %d\n", i, voxelIndicesCounts[i]);
//     }
// #endif

    return voxels;
}