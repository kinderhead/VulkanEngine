#pragma once

#include "utils.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "Datatypes.hpp"

class Pipeline
{
    vki::DescriptorSetLayout descriptorLayout;
    vki::PipelineLayout layout;

    vector<vki::Buffer> uniformBuffers;
    vector<vki::DeviceMemory> uniformBuffersMemory;
    vector<void *> uniformBuffersMapped;

    vk::DeviceSize uboSize;

    vki::DescriptorPool descriptorPool;
    vki::DescriptorSets descriptorSets;
public:
    Renderer* renderer;

    vki::Pipeline handle;

    shared_ptr<RenderPass> renderPass;

    Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders, VertexDefinition vertexDef, vk::DeviceSize uboSize);

    void setUBO(void* ubo);
    void bind(vki::CommandBuffer& cmds);
};
