cmake_minimum_required(VERSION 3.25.0)

project(VulkanEngine VERSION 0.1.0 LANGUAGES C CXX)

include(glsl-shaders.cmake)

add_subdirectory(Vulkan-Hpp)
add_subdirectory(Vulkan-Hpp/Vulkan-Headers)
add_subdirectory(Vulkan-Hpp/glm)

set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Build support for Wayland" FORCE)
add_subdirectory(Vulkan-Hpp/glfw)

find_program(GLSLANGVALIDATOR_EXE "glslangValidator")
mark_as_advanced(FORCE GLSLANGVALIDATOR_EXE)

add_subdirectory(VulkanEngine)
add_subdirectory(Testing)
