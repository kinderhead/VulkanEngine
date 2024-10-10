#pragma once

#include "utils.hpp"

class Renderer; // Forward declaration

class RenderPass
{
    vki::RenderPass handle;
public:
    Renderer* renderer;
    
    RenderPass(Renderer* renderer);

private:
    friend class Pipeline;
    friend class SwapChain;
};
