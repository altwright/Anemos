#pragma once
#include "vkstate.h"

void destroyVkState(VkState *vkstate);
void destroySwapchainSupportDetails(SwapchainSupportDetails *details);
void destroySwapchainDetails(VkDevice device, SwapchainDetails *swapchainDetails);
void destroyFramebuffers(VkDevice device, Framebuffers *framebuffers);