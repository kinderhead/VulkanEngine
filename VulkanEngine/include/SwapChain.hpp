#pragma once

#include "utils.hpp"

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Renderer; // Forward declaration
class RenderPass;

class SwapChain
{
    vki::SwapchainKHR handle;

    vector<vk::Image> images;
    vector<vki::ImageView> imageViews;
    vector<vki::Framebuffer> frameBuffers;
public:
    vk::Format imageFormat;
    vk::Extent2D extent;
    Renderer* renderer;

    SwapChain(Renderer* renderer);

    void populateFramebuffers(shared_ptr<RenderPass> renderPass);
private:
    SwapChainSupportDetails querySwapChainSupport(vki::PhysicalDevice device);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
};
