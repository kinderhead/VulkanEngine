#pragma once

#include "utils.hpp"
#include "SwapChain.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"

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
    vki::SurfaceKHR surface;

    vki::Queue graphicsQueue;
    vki::Queue presentQueue;

    GLFWwindow* window;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
public:
    vki::Device device;

    shared_ptr<Shader> basicVertShader;
    shared_ptr<Shader> basicFragShader;
    shared_ptr<Pipeline> basicPipeline;

    unique_ptr<SwapChain> swapChain;

    bool enableDebugLogs = true;

    Renderer(string title, GLFWwindow* window);

    void log(string txt);

private:
    // Average C++ destruct order error
    vki::CommandPool commandPool;
    vector<vki::CommandBuffer> commandBuffers;

    QueueFamilyIndices findQueueFamilies(vki::PhysicalDevice device);

    friend class SwapChain;
};
