cmake_minimum_required(VERSION 3.25.0)

project(VulkanEngine VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(Vulkan-Hpp)
add_subdirectory(Vulkan-Hpp/Vulkan-Headers)
add_subdirectory(Vulkan-Hpp/glm)
add_subdirectory(Vulkan-Hpp/glfw)

add_subdirectory(VulkanEngine)
add_subdirectory(Testing)
