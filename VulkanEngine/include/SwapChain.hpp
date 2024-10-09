#pragma once

#include "utils.hpp"

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Renderer; // Forward declaration

class SwapChain
{
    vki::SwapchainKHR handle;
    vector<vk::Image> images;
    vk::Format imageFormat;
    vk::Extent2D extent;
public:
    Renderer* renderer;
    
    SwapChain(Renderer* renderer);
private:
    SwapChainSupportDetails querySwapChainSupport(vki::PhysicalDevice device);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
};
