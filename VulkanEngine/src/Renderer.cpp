#include "Renderer.hpp"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(const vki::PhysicalDevice& device)
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

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

Renderer::Renderer() : instance({}), device({}), physicalDevice({}), graphicsQueue({})
{
    // Init
    log("Initializing Vulkan");

    auto info = vk::ApplicationInfo("Gaming", VK_MAKE_VERSION(1, 1, 0), "VulkanEngine", VK_MAKE_VERSION(1, 1, 0), VK_API_VERSION_1_3);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto createInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &info, 0, nullptr, glfwExtensionCount, glfwExtensions);
    instance = vki::Instance(ctx, createInfo);

    // Pick device
    auto devices = instance.enumeratePhysicalDevices();
    if (!devices.size())
    {
        throw std::runtime_error("No Vulkan devices found");
    }

    bool found = false;
    for (const auto& i : devices)
    {
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

    float queuePriority = 1;
    auto devQueueInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), findQueueFamilies(physicalDevice).graphicsFamily.value(), 1, &queuePriority);
    auto devInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), 1, &devQueueInfo);
    auto devFeatures = vk::PhysicalDeviceFeatures();
    devInfo.pEnabledFeatures = &devFeatures;
    devInfo.enabledExtensionCount = 0;

    device = physicalDevice.createDevice(devInfo);
    graphicsQueue = device.getQueue(findQueueFamilies(physicalDevice).graphicsFamily.value(), 0);
}

void Renderer::log(string txt)
{
    if (enableDebugLogs)
    {
        cout << txt << "\n";
    }
}
