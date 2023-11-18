#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "config.h"
#include "vkstate.h"
#include "model.h"
#include "vkbuffer.h"

int main(int, char**){
    UserConfig userConfig = {};
    if (!loadUserConfig(CONFIG_FILE, &userConfig)){
        fprintf(stderr, "Failed to load complete user config\n");
        exit(EXIT_FAILURE);
    }

    Window window = {};
    if (!createWindow(userConfig.window.width, userConfig.window.height, TITLE, &window)){
        fprintf(stderr, "Failed to create GLFW window!\n");
        exit(EXIT_FAILURE);
    }

    VulkanState vk = initVulkanState(&window, &userConfig);

    Model cube = loadModel("./models/cube.glb");

    size_t bytesCount = 0;
    unsigned char *mappedBuffer = (unsigned char*)vk.stagingBuffer.info.pMappedData;
    memcpy(mappedBuffer, cube.vertices, sizeof(Vertex)*cube.verticesCount);
    bytesCount += sizeof(Vertex)*cube.verticesCount;
    mappedBuffer += bytesCount;
    memcpy(mappedBuffer, cube.indices, sizeof(u16)*cube.indicesCount);
    bytesCount += sizeof(u16)*cube.indicesCount;

    copyToDeviceBuffer(
        bytesCount, 
        vk.stagingBuffer.handle, 0, 
        vk.deviceBuffer.handle, 0, 
        vk.device, 
        vk.transferCommandPool, 
        vk.transferQueue);

    while (!glfwWindowShouldClose(window.handle)){
        glfwPollEvents();
    }

    freeModel(&cube);
    destroyWindow(&window);
    destroyVulkanState(&vk);

    return 0;
}
