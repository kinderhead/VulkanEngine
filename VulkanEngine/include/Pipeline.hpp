#pragma once

#include "utils.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "Datatypes.hpp"
#include "UniformSet.hpp"

class Pipeline
{
    vki::DescriptorSetLayout descriptorLayout;

    vk::DeviceSize uboSize;

    vector<shared_ptr<UniformSet>> uniforms;
    int uniformSetsThisFrame;
public:
    Renderer* renderer;

    vki::Pipeline handle;
    vki::PipelineLayout layout;

    vk::Viewport viewport;
    vk::Rect2D scissor;

    Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders, VertexDefinition vertexDef, vk::DeviceSize uboSize);

    void beginFrame();
    void bind(vki::CommandBuffer& cmds);

    shared_ptr<UniformSet> getUniformSet();
};
