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

    cgltf_free(surfaceData);
    cgltf_free(characterData);

    return sceneInfo;
}
