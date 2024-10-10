#pragma once

#include "utils.hpp"

class Renderer; // Forward declaration

class Shader
{
    Renderer* renderer;

    vki::ShaderModule handle;
public:
    vk::ShaderStageFlagBits type;
    
    Shader(Renderer* renderer, string filename, vk::ShaderStageFlagBits type);

    vk::PipelineShaderStageCreateInfo getStageInfo();
};
