#pragma once

#include "utils.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"
#include "Datatypes.hpp"

class Pipeline
{
    vki::PipelineLayout layout;
public:
    Renderer* renderer;

    vki::Pipeline handle;

    shared_ptr<RenderPass> renderPass;
        
    Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders, VertexDefinition vertexDef);
};
