#pragma once

#include "utils.hpp"

class Pipeline; // Forward declaration

class UniformSet
{
    vki::DescriptorPool descriptorPool;
    
    vector<vki::Buffer> uniformBuffers;
    vector<vki::DeviceMemory> uniformBuffersMemory;
    vector<void*> uniformBuffersMapped;
    vector<vki::DescriptorSet> descriptorSets;

    vk::DeviceSize uboSize;
public:
    Pipeline* pipeline;

    UniformSet(Pipeline* pipeline, vki::DescriptorSetLayout& descriptorLayout, vk::DeviceSize uboSize);

    template <typename T>
    void setUBO(T& ubo);
    void setUBO(void* ubo);

    void bind(vki::CommandBuffer& cmds);

    template <typename T>
    void bindAndSetUBO(T& ubo, vki::CommandBuffer& cmds);
};

template <typename T>
inline void UniformSet::setUBO(T& ubo)
{
    setUBO(&ubo);
}

template <typename T>
void UniformSet::bindAndSetUBO(T& ubo, vki::CommandBuffer& cmds)
{
    setUBO(ubo);
    bind(cmds);
}

