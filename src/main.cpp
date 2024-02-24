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
#include "timing.h"

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

    SceneInfo scene = loadSceneToDevice(
        "./models/surface.glb",
        "./models/pompeii.glb",
        vk.stagingBuffer,
        vk.deviceBuffer,
        vk.device,
        vk.allocator,
        *vk.graphicsCmdPools,
        vk.graphicsQueue);

    size_t uniformBufferOffset = vk.physicalDevice.properties.limits.minUniformBufferOffsetAlignment;
    DescriptorSets descriptorSets = allocateDescriptorSets(vk.device, vk.descriptorSetLayout, vk.descriptorPool);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo = {};
        uniformBufferInfo.buffer = vk.uniformBuffer.handle;
        uniformBufferInfo.offset = i*uniformBufferOffset;
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
        surfaceTexDesc.imageView = scene.surfaceModelInfo.tex.view;
        surfaceTexDesc.sampler = vk.sampler;

        VkDescriptorImageInfo characterTexDesc = surfaceTexDesc;
        characterTexDesc.imageView = scene.characterModelInfo.tex.view;

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
    s64 prevTime_ns = getCurrentTime_ns();

    Character character = {.pos = {0.0f, 3.0f, 0.0f}, .vel_m_s = GLM_VEC3_ZERO_INIT};

    u32 currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle))
    {
        s64 currentTime_ns = getCurrentTime_ns();
        s64 timeDiff_ns = currentTime_ns - prevTime_ns;

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
        glm_mat4_mul_avx(projection.matrix, view.matrix, pushConstant.viewProjection);

        updateCharacterPhysics(&character, timeDiff_ns);
        applyCharacterSurfaceCollision(&character, &scene.surfaceVoxels);

        updateUniformBuffer(
            &vk.uniformBuffer, 
            currentFrame*uniformBufferOffset, 
            &scene.surfaceModelInfo);

        mat4 characterWorldMatrix = {};
        glm_translate_make(characterWorldMatrix, character.pos);
        glm_mat4_mul_avx(scene.characterModelInfo.modelMatrix, characterWorldMatrix, characterWorldMatrix);

        updateUniformBuffer(
            &vk.uniformBuffer, 
            currentFrame*uniformBufferOffset + sizeof(mat4), 
            characterWorldMatrix);

        recordModelDrawCommand(
            graphicsCmdBuffers[currentFrame],
            vk.renderPass,
            vk.framebuffers.handles[imageIndex],
            vk.graphicsPipeline,
            vk.swapchain.extent,
            pushConstant,
            descriptorSets.handles[currentFrame],
            vk.deviceBuffer.handle,
            scene.vtxBufOffset,
            scene.idxBufOffset,
            scene.drawCmdsOffset, scene.drawCmdsCount);

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
            
        prevTime_ns = currentTime_ns;

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        glfwPollEvents();
    }

    vkDeviceWaitIdle(vk.device);

    vkDestroyImageView(vk.device, scene.surfaceModelInfo.tex.view, NULL);
    vmaDestroyImage(vk.allocator, scene.surfaceModelInfo.tex.handle, scene.surfaceModelInfo.tex.alloc);
    vkDestroyImageView(vk.device, scene.characterModelInfo.tex.view, NULL);
    vmaDestroyImage(vk.allocator, scene.characterModelInfo.tex.handle, scene.characterModelInfo.tex.alloc);

    destroyWindow(&window);
    destroyVulkanState(&vk);

    freeSceneInfo(&scene);

    return 0;
}
