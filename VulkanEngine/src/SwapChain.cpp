#include "SwapChain.hpp"
#include "Renderer.hpp"


SwapChain::SwapChain(Renderer* renderer) : handle({}), renderer(renderer)
{
    // Init swap chain
    auto indices = renderer->findQueueFamilies(renderer->physicalDevice);
    vector<uint32_t> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    auto swapChainSupport = querySwapChainSupport(renderer->physicalDevice);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto swapChainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), renderer->surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, chooseSwapExtent(swapChainSupport.capabilities), 1, vk::ImageUsageFlagBits::eColorAttachment);

    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapChainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        swapChainInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainInfo.presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    swapChainInfo.clipped = true;
    swapChainInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

    try
    {
        handle = renderer->device.createSwapchainKHR(swapChainInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error making swap chain: " + string(err.what()));
    }

    images = handle.getImages();
    imageFormat = surfaceFormat.format;
    extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Make image views
    for (size_t i = 0; i < images.size(); i++)
    {
        vk::ImageViewCreateInfo info = {};
        info.image = images[i];
        info.viewType = vk::ImageViewType::e2D;
        info.format = imageFormat;
        info.components.r = vk::ComponentSwizzle::eIdentity;
        info.components.g = vk::ComponentSwizzle::eIdentity;
        info.components.b = vk::ComponentSwizzle::eIdentity;
        info.components.a = vk::ComponentSwizzle::eIdentity;
        info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        try
        {
            imageViews.push_back(renderer->device.createImageView(info));
        }
        catch (vk::SystemError err)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void SwapChain::populateFramebuffers(shared_ptr<RenderPass> renderPass)
{
    for (const auto& i : imageViews)
    {
        vk::ImageView attachments[] = { i };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = renderPass->handle;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        try
        {
            frameBuffers.push_back(renderer->device.createFramebuffer(framebufferInfo));
        }
        catch (vk::SystemError err)
        {
            throw std::runtime_error("Error making framebuffer");
        }
    }
}

SwapChainSupportDetails SwapChain::querySwapChainSupport(vki::PhysicalDevice device)
{
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(renderer->surface);
    details.formats = device.getSurfaceFormatsKHR(renderer->surface);
    details.presentModes = device.getSurfacePresentModesKHR(renderer->surface);
    return details;
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
    {
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    for (const auto& i : availableFormats)
    {
        if (i.format == vk::Format::eB8G8R8A8Unorm && i.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return i;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes)
{
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for (const auto& i : availablePresentModes)
    {
        if (i == vk::PresentModeKHR::eMailbox)
        {
            return i;
        }
        else if (i == vk::PresentModeKHR::eImmediate)
        {
            bestMode = i;
        }
    }

    return bestMode;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width;
        int height;
        glfwGetWindowSize(renderer->window, &width, &height);

        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}