#pragma once

#include "utils.hpp"
#include "Shader.hpp"
#include "RenderPass.hpp"

class Pipeline
{
    vki::PipelineLayout layout;
    vki::Pipeline handle;
public:
    Renderer* renderer;
    
    shared_ptr<RenderPass> renderPass;
        
    Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders);
};
