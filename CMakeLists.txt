cmake_minimum_required(VERSION 3.25.0)

project(VulkanEngine VERSION 0.1.0 LANGUAGES C CXX)

include(cmake/glsl-shaders.cmake)
include(cmake/CPM.cmake)

find_program(GLSLANGVALIDATOR_EXE "glslangValidator")
mark_as_advanced(FORCE GLSLANGVALIDATOR_EXE)

add_subdirectory(VulkanEngine)
add_subdirectory(Testing)
