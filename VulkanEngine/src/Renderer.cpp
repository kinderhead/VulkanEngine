#include "Renderer.hpp"
#include "Model.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<BasicVertex> rectangleVertices = {
    {{0.0f, 0.0f}},
    {{1.0f, 0.0f}},
    {{1.0f, 1.0f}},
    {{0.0f, 1.0f}}
};

const std::vector<uint32_t> rectangleIndices = {
    0, 1, 2, 2, 3, 0
};

Renderer::Renderer(string title, GLFWwindow* window) : instance({}), device({}), physicalDevice({}), graphicsQueue({}), presentQueue({}), surface({}), window(window), commandPool({})
{
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    // Init
    log("Initializing Vulkan");

    auto info = vk::ApplicationInfo(title.c_str(), VK_MAKE_VERSION(1, 1, 0), "VulkanEngine", VK_MAKE_VERSION(1, 1, 0), VK_API_VERSION_1_3);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto createInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &info, 1, validationLayers.data(), glfwExtensionCount, glfwExtensions);
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
        queueCreateInfos.push_back({ {}, i, 1, &queuePriority });
    }

    if (indices.graphicsFamily.value() == indices.presentFamily.value())
    {
        queueCreateInfos.pop_back();
    }

    auto devInfo = vk::DeviceCreateInfo({}, static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data());
    auto devFeatures = vk::PhysicalDeviceFeatures();
    devInfo.pEnabledFeatures = &devFeatures;
    devInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    devInfo.ppEnabledExtensionNames = deviceExtensions.data();

    device = physicalDevice.createDevice(devInfo);
    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);

    // Make swapchain'
    swapChain = make_unique<SwapChain>(this);
    log("Created swap chain");

    // Do things
    basicVertShader = make_shared<Shader>(this, "VulkanEngine/shaders/shader.vert.spv", vk::ShaderStageFlagBits::eVertex);
    basicFragShader = make_shared<Shader>(this, "VulkanEngine/shaders/shader.frag.spv", vk::ShaderStageFlagBits::eFragment);
    log("Compiled shaders");

    renderPass = make_shared<RenderPass>(this);
    log("Created base render pass");

    basicPipeline = make_shared<Pipeline>(this, vector<shared_ptr<Shader>>{ basicVertShader, basicFragShader }, BasicVertex::getVertexDefinition(), sizeof(BasicUBO));
    log("Created render pipeline");

    swapChain->populateFramebuffers(renderPass);
    log("Created framebuffers");

    // Setup commands
    vk::CommandPoolCreateInfo poolInfo = {vk::CommandPoolCreateFlagBits::eResetCommandBuffer};
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    try
    {
        commandPool = device.createCommandPool(poolInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error making command pool");
    }

    // Make models
    rectangle = make_shared<Model<BasicVertex>>(this, rectangleVertices, rectangleIndices);
    log("Created typical models");

    // More commands
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    
    try
    {
        commandBuffers = device.allocateCommandBuffers(allocInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error allocating command buffers");
    }

    log("Created command buffers");

    try
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            imageAvailableSemaphores.push_back(device.createSemaphore({}));
            renderFinishedSemaphores.push_back(device.createSemaphore({}));
            inFlightFences.push_back(device.createFence({ vk::FenceCreateFlagBits::eSignaled }));
        }
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error creating sync objects");
    }

    log("Created syncronization objects");
}

void Renderer::beginFrame()
{
    auto fence = *inFlightFences[currentFlightFrame];
    if (device.waitForFences(vk::ArrayProxy<vk::Fence>(1, &fence), true, numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
    {
        throw runtime_error("Error waiting for device");
    }

    try
    {
        auto res = swapChain->handle.acquireNextImage(numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFlightFrame]);
        currentFrameImageIndex = res.second;
    }
    catch (vk::OutOfDateKHRError err)
    {
        recreateSwapChain();
        return;
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error getting frame");
    }

    device.resetFences(vk::ArrayProxy<vk::Fence>(1, &fence));
    commandBuffers[currentFlightFrame].reset();

    commandBuffers[currentFlightFrame].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    commandBuffers[currentFlightFrame].beginRenderPass(renderPass->getBeginInfo(swapChain->framebuffers[currentFrameImageIndex]), vk::SubpassContents::eInline);

    basicPipeline->beginFrame();
}

void Renderer::endFrame()
{
    commandBuffers[currentFlightFrame].endRenderPass();
    commandBuffers[currentFlightFrame].end();

    auto fence = *inFlightFences[currentFlightFrame];

    vk::SubmitInfo submitInfo = {};

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFlightFrame] };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;

    auto buf = *commandBuffers[currentFlightFrame];
    submitInfo.pCommandBuffers = &buf;

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFlightFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try
    {
        graphicsQueue.submit(submitInfo, inFlightFences[currentFlightFrame]);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error drawing :(");
    }

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = { swapChain->handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &currentFrameImageIndex;
    presentInfo.pResults = nullptr;

    vk::Result resultPresent;
    try
    {
        resultPresent = presentQueue.presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError err)
    {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error presenting frame");
    }

    if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }

    currentFlightFrame = (currentFlightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::log(string txt)
{
    if (enableDebugLogs)
    {
        cout << txt << "\n";
    }
}

void Renderer::stop()
{
    device.waitIdle();
}

void Renderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vki::Buffer& buffer, vki::DeviceMemory& bufferMemory)
{
    auto bufferInfo = vk::BufferCreateInfo({}, size, usage);

    try
    {
        buffer = device.createBuffer(bufferInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error creating buffer");
    }

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    try
    {
        bufferMemory = device.allocateMemory(allocInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error allocating buffer memory");
    }

    buffer.bindMemory(bufferMemory, 0);
}

void Renderer::copyBuffer(vki::Buffer& src, vki::Buffer& dest, vk::DeviceSize size)
{
    auto allocInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    auto cmdBuffs = device.allocateCommandBuffers(allocInfo);

    auto beginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuffs[0].begin(beginInfo);

    cmdBuffs[0].copyBuffer(src, dest, vk::BufferCopy(0, 0, size));

    cmdBuffs[0].end();

    vk::CommandBuffer tempBuf = cmdBuffs[0];
    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tempBuf;

    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Could not find GPU memory type");
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

void Renderer::recreateSwapChain()
{
    int width = 0, height = 0;
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();

    swapChain.reset(nullptr);
    swapChain = make_unique<SwapChain>(this);
    swapChain->populateFramebuffers(renderPass);
}

void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Renderer::drawRectangle(int x, int y, int width, int height, float rotation, vec4 color)
{
    auto ubo = getNewUBO();
    ubo.model = translate(mat4(1), vec3(x, y, 0)) * rotate(mat4(1), rotation, vec3(0, 0, 1)) * scale(mat4(1), vec3(width, height, 0));
    ubo.color = color;
    
    basicPipeline->bind(commandBuffers[currentFlightFrame]);
    basicPipeline->getUniformSet()->bindAndSetUBO(ubo, commandBuffers[currentFlightFrame]);
    rectangle->draw(commandBuffers[currentFlightFrame]);
}

BasicUBO Renderer::getNewUBO()
{
    return {mat4(1), lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)), ortho(0.0f, (float)swapChain->extent.width, 0.0f, (float)swapChain->extent.height, -1000.0f, 1000.0f)};
}
