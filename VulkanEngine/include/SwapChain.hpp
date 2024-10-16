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
    vector<vk::Image> images;
    vector<vki::ImageView> imageViews;
public:
    vk::Format imageFormat;
    vk::Extent2D extent;

    vector<vki::Framebuffer> framebuffers;

    vki::SwapchainKHR handle;

    Renderer* renderer;

    SwapChain(Renderer* renderer);

    void populateFramebuffers(shared_ptr<RenderPass> renderPass);
private:
    SwapChainSupportDetails querySwapChainSupport(vki::PhysicalDevice device);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
};
