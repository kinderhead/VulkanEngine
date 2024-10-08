#pragma once

#include "utils.hpp"

class Renderer
{
    vki::Context ctx;
    vki::Instance instance;
    vki::PhysicalDevice physicalDevice;
    vki::Device device;
    vki::Queue graphicsQueue;
public:
    bool enableDebugLogs = true;
    
    Renderer();

    void log(string txt);
};
