#pragma once

#include "utils.hpp"
#include "SwapChain.hpp"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Renderer
{
    vki::Context ctx;
    vki::Instance instance;

    vki::PhysicalDevice physicalDevice;
    vki::Device device;
    vki::SurfaceKHR surface;

    unique_ptr<SwapChain> swapChain;

    vki::Queue graphicsQueue;
    vki::Queue presentQueue;

    GLFWwindow* window;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
public:
    bool enableDebugLogs = true;
    
    Renderer(string title, GLFWwindow* window);

    void log(string txt);

private:
    QueueFamilyIndices findQueueFamilies(vki::PhysicalDevice device);

    friend class SwapChain;
};
