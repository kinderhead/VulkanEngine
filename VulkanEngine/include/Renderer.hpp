#pragma once

#include "utils.hpp"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Renderer
{
    vki::Context ctx;
    vki::Instance instance;

    vki::PhysicalDevice physicalDevice;
    vki::Device device;
    vki::SurfaceKHR surface;

    vki::SwapchainKHR swapChain;
    vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;

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
    SwapChainSupportDetails querySwapChainSupport(vki::PhysicalDevice device);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
};
