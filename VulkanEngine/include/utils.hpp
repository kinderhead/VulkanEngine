#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <optional>
#include <set>
#include <vector>
#include <fstream>
#include <array>
#include <cmath>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;
namespace vki = vk::raii;

inline vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not read file: " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
