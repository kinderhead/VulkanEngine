cmake_minimum_required(VERSION 3.25.0)

add_library(VulkanEngine
    src/VulkanEngine.cpp
    src/Window.cpp
    src/Renderer.cpp

    include/utils.hpp
    include/Window.hpp
    include/Renderer.hpp
)

target_include_directories(VulkanEngine PUBLIC include ../Vulkan-Hpp/vulkan)

set_property(TARGET VulkanEngine PROPERTY CXX_STANDARD 23)

target_link_libraries(VulkanEngine PUBLIC Vulkan::Headers glfw glm)
