cmake_minimum_required(VERSION 3.25.0)
project(Testing VERSION 0.1.0 LANGUAGES C CXX)

add_library(Testing
    src/main.cpp
)

target_link_libraries(Testing PUBLIC VulkanEngine)
