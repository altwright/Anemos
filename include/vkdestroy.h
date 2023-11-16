#pragma once
#include "vkstate.h"

void _destroyVkState(VkState *vkstate);
void destroySwapchainSupportDetails(SwapchainSupportDetails *details);
void destroySwapchain(VkDevice device, SwapchainDetails *swapchainDetails);
void destroyFramebuffers(VkDevice device, Framebuffers *framebuffers);
void destroyImage(VkDevice device, Image *image);