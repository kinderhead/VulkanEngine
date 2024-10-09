#include "Renderer.hpp"

Renderer::Renderer(string title, GLFWwindow* window) : instance({}), device({}), physicalDevice({}), graphicsQueue({}), presentQueue({}), surface({}), window(window), swapChain({})
{
    // Init
    log("Initializing Vulkan");

    auto info = vk::ApplicationInfo("Gaming", VK_MAKE_VERSION(1, 1, 0), "VulkanEngine", VK_MAKE_VERSION(1, 1, 0), VK_API_VERSION_1_3);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto createInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &info, 0, nullptr, glfwExtensionCount, glfwExtensions);
    instance = vki::Instance(ctx, createInfo);

    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), window, nullptr, &rawSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = vki::SurfaceKHR(instance, rawSurface);

    // Pick device
    auto devices = instance.enumeratePhysicalDevices();
    if (!devices.size())
    {
        throw std::runtime_error("No Vulkan devices found");
    }

    bool found = false;
    for (const auto& i : devices)
    {
        // Hope that the swap chain works
        if (findQueueFamilies(i).isComplete())
        {
            physicalDevice = i;
            log("Using device: " + string(physicalDevice.getProperties().deviceName));
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw std::runtime_error("No compatible Vulkan devices found");
    }

    // Setup device
    auto indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1;

    vector<uint32_t> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    for (uint32_t i : queueFamilyIndices)
    {
        queueCreateInfos.push_back({ vk::DeviceQueueCreateFlags(), i, 1, &queuePriority });
    }

    auto devInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data());
    auto devFeatures = vk::PhysicalDeviceFeatures();
    devInfo.pEnabledFeatures = &devFeatures;
    devInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    devInfo.ppEnabledExtensionNames = deviceExtensions.data();

    device = physicalDevice.createDevice(devInfo);
    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);

    // Make swapchain
    auto swapChainSupport = querySwapChainSupport(physicalDevice);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto swapChainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(), surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, chooseSwapExtent(swapChainSupport.capabilities), 1, vk::ImageUsageFlagBits::eColorAttachment);

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
        swapChain = device.createSwapchainKHR(swapChainInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error making swap chain: " + string(err.what()));
    }

    swapChainImages = swapChain.getImages();
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);
}

void Renderer::log(string txt)
{
    if (enableDebugLogs)
    {
        cout << txt << "\n";
    }
}

QueueFamilyIndices Renderer::findQueueFamilies(vki::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, surface))
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails Renderer::querySwapChainSupport(vki::PhysicalDevice device)
{
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);
    return details;
}

vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes)
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

vk::Extent2D Renderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
