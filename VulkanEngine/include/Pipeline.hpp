#pragma once

#include "utils.hpp"
#include "Shader.hpp"

class Pipeline
{
    vki::PipelineLayout layout;
public:
    Renderer* renderer;
        
    Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders);
};
