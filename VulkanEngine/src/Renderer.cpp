#include "Renderer.hpp"

Renderer::Renderer(string title, GLFWwindow* window) : instance({}), device({}), physicalDevice({}), graphicsQueue({}), presentQueue({}), surface({}), window(window), commandPool({})
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
    swapChain = make_unique<SwapChain>(this);
    log("Created swap chain");

    // Do things
    basicVertShader = make_shared<Shader>(this, "VulkanEngine/shaders/shader.vert.spv", vk::ShaderStageFlagBits::eVertex);
    basicFragShader = make_shared<Shader>(this, "VulkanEngine/shaders/shader.frag.spv", vk::ShaderStageFlagBits::eFragment);
    log("Compiled shaders");

    basicPipeline = make_shared<Pipeline>(this, vector<shared_ptr<Shader>>{ basicVertShader, basicFragShader });
    log("Created render pipeline");

    swapChain->populateFramebuffers(basicPipeline->renderPass);
    log("Created framebuffers");

    // Setup commands
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    try
    {
        commandPool = device.createCommandPool(poolInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error making command pool");
    }

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t) swapChain->framebuffers.size();

    vk::ClearValue clearColor = { array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };

    try
    {
        commandBuffers = device.allocateCommandBuffers(allocInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error allocating command buffers");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
        commandBuffers[i].begin(beginInfo);
        commandBuffers[i].beginRenderPass(basicPipeline->renderPass->getBeginInfo(swapChain->framebuffers[i], &clearColor), vk::SubpassContents::eInline);

        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, basicPipeline->handle);
        commandBuffers[i].draw(3, 1, 0, 0);

        commandBuffers[i].endRenderPass();
        commandBuffers[i].end();
    }
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
