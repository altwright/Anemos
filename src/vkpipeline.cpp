#include "vkpipeline.h"
#include <stdio.h>
#include <stdlib.h>

VkRenderPass createRenderPass(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice,
    const SwapchainDetails *swapchain,
    const Image *depthImage,
    const Image *samplingImage,
    VkSampleCountFlagBits samplingCount)
{
    VkAttachmentDescription samplingAttachmentDesc = {};
    samplingAttachmentDesc.format = samplingImage->format;
    samplingAttachmentDesc.samples = samplingCount;
    samplingAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    samplingAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    samplingAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    samplingAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    samplingAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    samplingAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachmentDesc = {};
    depthAttachmentDesc.format = depthImage->format;
    depthAttachmentDesc.samples = samplingCount;
    depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription resolveAttachmentDesc = {};
    resolveAttachmentDesc.format = swapchain->format;
    resolveAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolveAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolveAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolveAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference samplingAttachmentRef = {};
    samplingAttachmentRef.attachment = 0;//References the first attachment description by index
    samplingAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //Transitions layout of attachment to this during subpass

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolveAttachmentRef = {};
    resolveAttachmentRef.attachment = 2;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //Subpasses are subsequent rendering operations that depend on the contents of 
    //framebuffers in previous passes
    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &samplingAttachmentRef;
    subpassDesc.pResolveAttachments = &resolveAttachmentRef;
    //The index of the attachment in this array is directly 
    //referenced from the fragment shader with the 
    //layout(location = 0) out vec4 outColor directive!
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;//Index to the only subpass. Must be higher than srcSubpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //The above two fields specify the operations to wait on and the stages in which 
    //these operations occur. We need to wait for the swap chain to finish reading 
    //from the image before we can access it. This can be accomplished by waiting on 
    //the color attachment output stage itself.
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //The operations that should wait on this are in the color attachment stage 
    //and involve the writing of the color attachment. These settings will prevent 
    //the transition from happening until it’s actually necessary (and allowed): 
    //when we want to start writing colors to it.
    
    u32 attachmentDescriptionsCount = 3;
    VkAttachmentDescription attachmentDescriptions[attachmentDescriptionsCount] = {
        samplingAttachmentDesc, 
        depthAttachmentDesc, 
        resolveAttachmentDesc};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachmentDescriptionsCount;
    renderPassInfo.pAttachments = attachmentDescriptions;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass)){
        printf("Failed to create Render Pass\n");
        exit(EXIT_FAILURE);
    }

    return renderPass;
}