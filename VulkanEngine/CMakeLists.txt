cmake_minimum_required(VERSION 3.25.0)

add_library(VulkanEngine
    src/VulkanEngine.cpp
    src/Window.cpp
    src/Renderer.cpp
    src/SwapChain.cpp
    src/Shader.cpp
    src/Pipeline.cpp
    src/RenderPass.cpp
    src/UniformSet.cpp

    include/utils.hpp
    include/Window.hpp
    include/Renderer.hpp
    include/SwapChain.hpp
    include/Shader.hpp
    include/Pipeline.hpp
    include/RenderPass.hpp
    include/Datatypes.hpp
    include/Model.hpp
    include/UniformSet.hpp
)

target_glsl_shaders(
    VulkanEngine
    PUBLIC

    shaders/shader.frag
    shaders/shader.vert
    shaders/elipse.frag
    shaders/elipse.vert

    COMPILE_OPTIONS --target-env vulkan1.3
)

target_include_directories(VulkanEngine PUBLIC include ../Vulkan-Hpp/vulkan)

set_property(TARGET VulkanEngine PROPERTY CXX_STANDARD 23)

find_package(Vulkan REQUIRED)
CPMAddPackage("https://github.com/g-truc/glm/archive/refs/heads/master.zip")

target_link_libraries(VulkanEngine PUBLIC Vulkan::Vulkan glfw glm)
