cmake_minimum_required(VERSION 3.27.0)
project(anemos VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(anemos)

set_target_properties(anemos PROPERTIES
    COMPILE_WARNING_AS_ERROR ON
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")

set(GLFW_USE_WAYLAND ON)

target_include_directories(anemos PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

add_subdirectory(src)
add_subdirectory(libs/glfw)
add_subdirectory(libs/cglm)
add_subdirectory(libs/VulkanMemoryAllocator)

find_package(Vulkan REQUIRED)

target_link_libraries(anemos PRIVATE
    glfw
    Vulkan::Vulkan
    cglm
    GPUOpen::VulkanMemoryAllocator
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
