#include "vkdescriptor.h"
#include <string.h>
#include "timing.h"
#include "int.h"

void updateUniformBuffer(void *mappedUniformBuffer, VkExtent2D swapchainExtent)
{
    static const s64 startTimeNs = SEC_TO_NS(START_TIME.tv_sec) + START_TIME.tv_nsec;

    timespec currentTime = {};
    if (clock_gettime(TIMING_CLOCK, &currentTime)){
        perror("Failed to get Current Time\n");
        exit(EXIT_FAILURE);
    }

    s64 currentTimeNs = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiffNs = currentTimeNs - startTimeNs;
    float rotationRadians = (glm_rad(90.0f) * timeDiffNs)/SEC_TO_NS(1);

    UniformBufferData ubo = {};
    glm_mat4_identity(ubo.model);
    vec3 rotationAxis = {0.0f, 0.0f, 1.0f};
    glm_rotate(ubo.model, rotationRadians, rotationAxis);
    vec3 eye = {2.0f, 2.0f, 2.0f};
    vec3 centre = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_lookat(eye, centre, up, ubo.view);
    glm_perspective(glm_rad(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f, ubo.projection);
    ubo.projection[1][1] *= -1;//Originally designed for OpenGL, so must be inverted

    memcpy(mappedUniformBuffer, &ubo, sizeof(UniformBufferData));
    //Push Constants are a more efficient way of transferring small data to the shaders
}