#include <stdio.h>
#include <stdlib.h>
#include "window.h"
#include "config.h"
#include "vkstate.h"
#include "model.h"

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

    Model model = loadModel("./models/cube.glb");

    while (!glfwWindowShouldClose(window.handle)){
        glfwPollEvents();
    }

    freeModel(&model);
    destroyWindow(&window);
    destroyVulkanState(&vk);

    return 0;
}
