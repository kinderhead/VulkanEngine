#pragma once

#include "utils.hpp"

class Renderer; // Forward declaration

class RenderPass
{
public:
    vki::RenderPass handle;
    Renderer* renderer;
    
    RenderPass(Renderer* renderer);

    vk::RenderPassBeginInfo getBeginInfo(vki::Framebuffer& framebuffer);
};
