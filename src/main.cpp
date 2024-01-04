#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "config.h"
#include "vkstate.h"
#include "model.h"
#include "vkmemory.h"
#include "vkswapchain.h"
#include "vkcommand.h"
#include "vkshader.h"
#include "input.h"
#include "controls.h"

int main(int, char**)
{
    UserConfig userConfig = {};
    if (!loadUserConfig(CONFIG_FILE, &userConfig))
    {
        fprintf(stderr, "Failed to load complete user config\n");
        exit(EXIT_FAILURE);
    }

    Window window = {};
    if (!initWindow(
        TITLE, 
        userConfig.window.width, 
        userConfig.window.height, 
        &window))
    {
        fprintf(stderr, "Failed to create Window!\n");
        exit(EXIT_FAILURE);
    }

    VulkanState vk = initVulkanState(&window, &userConfig);

    VkCommandBuffer graphicsCmdBuffers[MAX_FRAMES_IN_FLIGHT] = {};
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        graphicsCmdBuffers[i] = createPrimaryCommandBuffer(vk.device, vk.graphicsCmdPools[i]);
    }

    u8 *mappedBuffer = (u8*)vk.stagingBuffer.info.pMappedData;
    ModelInfo modelInfo = loadModelIntoStagingBuffer("./models/dead_cube.glb", mappedBuffer);
    size_t modelDataSize = 
        modelInfo.verticesDataSize + 
        modelInfo.indicesDataSize + 
        modelInfo.texCoordDataSize + 
        modelInfo.texWidth * modelInfo.texHeight * modelInfo.texChannels;

    copyToDeviceBuffer(
        modelDataSize,
        vk.stagingBuffer.handle, 0, 
        vk.deviceBuffer.handle, 0, 
        vk.device, 
        vk.transferCommandPool, 
        vk.transferQueue);

    DescriptorSets descriptorSets = allocateDescriptorSets(vk.device, vk.descriptorSetLayout, vk.descriptorPool);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo = {};
        uniformBufferInfo.buffer = vk.uniformBuffer.handle;
        uniformBufferInfo.offset = i*vk.physicalDevice.deviceProperties.limits.minUniformBufferOffsetAlignment;
        uniformBufferInfo.range = sizeof(mat4);

        VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet = descriptorSets.handles[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.pBufferInfo = &uniformBufferInfo;

        vkUpdateDescriptorSets(vk.device, 1, &descriptorWrite, 0, NULL);
    }

    CameraControls cam = cam_createControls();
    cam_setInputHandler(&cam, &window.inputHandler);

    PushConstant pushConstant = {};
    Matrix4 projection = cam_genProjectionMatrix(&cam, vk.swapchain.extent);
    u32 currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle))
    {
        cam_processInput(&window);

        vkWaitForFences(vk.device, 1, &vk.frameSyncers[currentFrame].inFlight, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;//Will refer to a VkImage in our swapchain images array
        VkResult result = vkAcquireNextImageKHR(
            vk.device, 
            vk.swapchain.handle, 
            UINT64_MAX, 
            vk.frameSyncers[currentFrame].imageAvailable, 
            VK_NULL_HANDLE, 
            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain(
                vk.allocator,
                vk.device,
                &vk.physicalDevice,
                vk.surface,
                window.handle,
                vk.renderPass,
                vk.physicalDevice.maxSamplingCount,
                &vk.swapchain,
                &vk.depthImage,
                &vk.samplingImage,
                &vk.framebuffers);
            
            projection = cam_genProjectionMatrix(&cam, vk.swapchain.extent);
            continue;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            printf("Failed to Acquire Next Swapchain Image: %d\n", result);
            exit(EXIT_FAILURE);
        }

        vkResetFences(vk.device, 1, &vk.frameSyncers[currentFrame].inFlight);
        vkResetCommandPool(vk.device, vk.graphicsCmdPools[currentFrame], 0);

        Matrix4 view = cam_genViewMatrix(&cam);
        glm_mat4_mul_sse2(projection.matrix, view.matrix, pushConstant.viewProjection);
        updateUniformBuffer(
            &vk.uniformBuffer, 
            currentFrame*vk.physicalDevice.deviceProperties.limits.minUniformBufferOffsetAlignment, 
            &modelInfo);

        recordModelDrawCommand(
            graphicsCmdBuffers[currentFrame],
            vk.renderPass,
            vk.framebuffers.handles[imageIndex],
            vk.graphicsPipeline,
            vk.swapchain.extent,
            pushConstant,
            descriptorSets.handles[currentFrame],
            vk.deviceBuffer.handle,
            0, modelInfo.verticesCount,
            modelInfo.verticesDataSize, modelInfo.indicesCount);

        submitDrawCommand(
            vk.graphicsQueue,
            graphicsCmdBuffers[currentFrame],
            vk.frameSyncers[currentFrame].imageAvailable,
            vk.frameSyncers[currentFrame].renderFinished,
            vk.frameSyncers[currentFrame].inFlight);

        result = presentSwapchain(
            vk.presentQueue,
            vk.frameSyncers[currentFrame].renderFinished,
            vk.swapchain.handle,
            imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || 
            result == VK_SUBOPTIMAL_KHR || 
            window.resizing)
        {
            recreateSwapchain(
                vk.allocator,
                vk.device,
                &vk.physicalDevice,
                vk.surface,
                window.handle,
                vk.renderPass,
                vk.physicalDevice.maxSamplingCount,
                &vk.swapchain,
                &vk.depthImage,
                &vk.samplingImage,
                &vk.framebuffers);

            projection = cam_genProjectionMatrix(&cam, vk.swapchain.extent);
            window.resizing = false;
        }
        else if (result != VK_SUCCESS)
        {
            printf("Failed to Present Swapchain Image\n");
            exit(EXIT_FAILURE);
        }
            
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        glfwPollEvents();
    }

    vkDeviceWaitIdle(vk.device);

    destroyWindow(&window);
    destroyVulkanState(&vk);

    return 0;
}
