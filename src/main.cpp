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
#include "scene.h"

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

    SceneInfo sceneInfo = loadSceneToDevice(
        "./models/surface.glb",
        "./models/pompeii.glb",
        vk.stagingBuffer,
        vk.deviceBuffer,
        vk.device,
        vk.allocator,
        *vk.graphicsCmdPools,
        vk.graphicsQueue);

    DescriptorSets descriptorSets = allocateDescriptorSets(vk.device, vk.descriptorSetLayout, vk.descriptorPool);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo = {};
        uniformBufferInfo.buffer = vk.uniformBuffer.handle;
        uniformBufferInfo.offset = i*vk.physicalDevice.properties.limits.minUniformBufferOffsetAlignment;
        uniformBufferInfo.range = sizeof(mat4);

        VkWriteDescriptorSet ubDescriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        ubDescriptorWrite.dstSet = descriptorSets.handles[i];
        ubDescriptorWrite.dstBinding = 0;
        ubDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubDescriptorWrite.dstArrayElement = 0;
        ubDescriptorWrite.descriptorCount = 1;
        ubDescriptorWrite.pBufferInfo = &uniformBufferInfo;

        VkDescriptorImageInfo surfaceTexDesc = {};
        surfaceTexDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        surfaceTexDesc.imageView = sceneInfo.surfaceModelInfo.tex.view;
        surfaceTexDesc.sampler = vk.sampler;

        VkDescriptorImageInfo characterTexDesc = surfaceTexDesc;
        characterTexDesc.imageView = sceneInfo.characterModelInfo.tex.view;

        VkDescriptorImageInfo texDescriptors[] = {surfaceTexDesc, characterTexDesc};
        VkWriteDescriptorSet texDescriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        texDescriptorWrite.dstSet = descriptorSets.handles[i];
        texDescriptorWrite.dstBinding = 1;
        texDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texDescriptorWrite.dstArrayElement = 0;
        texDescriptorWrite.descriptorCount = NUM_ELEMENTS(texDescriptors);
        texDescriptorWrite.pImageInfo = texDescriptors;

        VkWriteDescriptorSet descriptorWrites[] = {ubDescriptorWrite, texDescriptorWrite};

        vkUpdateDescriptorSets(vk.device, NUM_ELEMENTS(descriptorWrites), descriptorWrites, 0, NULL);
    }

    VkCommandBuffer graphicsCmdBuffers[MAX_FRAMES_IN_FLIGHT] = {};
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        graphicsCmdBuffers[i] = createPrimaryCommandBuffer(vk.device, vk.graphicsCmdPools[i]);
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
            currentFrame*vk.physicalDevice.properties.limits.minUniformBufferOffsetAlignment, 
            &sceneInfo.surfaceModelInfo);

        updateUniformBuffer(
            &vk.uniformBuffer, 
            currentFrame*vk.physicalDevice.properties.limits.minUniformBufferOffsetAlignment + sizeof(mat4), 
            &sceneInfo.characterModelInfo);

        recordModelDrawCommand(
            graphicsCmdBuffers[currentFrame],
            vk.renderPass,
            vk.framebuffers.handles[imageIndex],
            vk.graphicsPipeline,
            vk.swapchain.extent,
            pushConstant,
            descriptorSets.handles[currentFrame],
            vk.deviceBuffer.handle,
            sceneInfo.vtxBufOffset,
            sceneInfo.idxBufOffset,
            sceneInfo.drawCmdsOffset, sceneInfo.drawCmdsCount);

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

    vkDestroyImageView(vk.device, sceneInfo.surfaceModelInfo.tex.view, NULL);
    vmaDestroyImage(vk.allocator, sceneInfo.surfaceModelInfo.tex.handle, sceneInfo.surfaceModelInfo.tex.alloc);
    vkDestroyImageView(vk.device, sceneInfo.characterModelInfo.tex.view, NULL);
    vmaDestroyImage(vk.allocator, sceneInfo.characterModelInfo.tex.handle, sceneInfo.characterModelInfo.tex.alloc);

    destroyWindow(&window);
    destroyVulkanState(&vk);

    return 0;
}
