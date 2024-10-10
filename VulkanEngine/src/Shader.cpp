#include "Shader.hpp"

#include "Renderer.hpp"

Shader::Shader(Renderer* renderer, string filename, vk::ShaderStageFlagBits type) : handle({}), renderer(renderer), type(type)
{
    auto code = readFile(filename);
    handle = renderer->device.createShaderModule({ vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data()) });
}

vk::PipelineShaderStageCreateInfo Shader::getStageInfo()
{
    return vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), type, handle, "main");
}
