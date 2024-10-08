cmake_minimum_required(VERSION 3.25.0)
project(Testing VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Testing
    src/main.cpp
    src/TestWindow.cpp

    include/TestWindow.hpp
)

target_include_directories(Testing PUBLIC include)

set_property(TARGET Testing PROPERTY CXX_STANDARD 23)

target_link_libraries(Testing PUBLIC VulkanEngine)
