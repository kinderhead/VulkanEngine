#pragma once

#include "utils.hpp"
#include "SwapChain.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Datatypes.hpp"

#include "CDT.h"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

template <typename TVertex>
class Model; // Forward declaration
template <typename TVertex>
class DynamicModel;
class BaseModel;

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

    bool framebufferResized = false;

    uint32_t currentFrameImageIndex;

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

public:
    const int MAX_FRAMES_IN_FLIGHT = 2;

    long currentFlightFrame = 0;
    vki::Device device;

    shared_ptr<RenderPass> renderPass;

    vk::ClearValue clearColor = { array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };

    bool enableDebugLogs = true;

    Renderer(string title, GLFWwindow* window);

    void beginFrame();
    void endFrame();
    void stop();

    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vki::Buffer& buffer, vki::DeviceMemory& bufferMemory);
    template <typename T>
    void createBufferWithStaging(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vki::Buffer& buffer, vki::DeviceMemory& bufferMemory, const vector<T>& data);
    void copyBuffer(vki::Buffer& src, vki::Buffer& dest, vk::DeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    // Rendering
    void drawRectangle(int x, int y, int width, int height, float rotation = 0, vec4 color = {1, 1, 1, 1});
    void drawElipse(int x, int y, int width, int height, float rotation = 0, vec4 color = {1, 1, 1, 1});

    inline void drawPolygon(initializer_list<BasicVertex> points, int x = 0, int y = 0, int width = 1, int height = 1, float rotation = 0, vec4 color = {1, 1, 1, 1});
    void drawPolygon(vector<BasicVertex>& points, int x = 0, int y = 0, int width = 1, int height = 1, float rotation = 0, vec4 color = {1, 1, 1, 1});

    template <typename T>
    void drawModel(shared_ptr<BaseModel> model, shared_ptr<Pipeline> pipeline, T ubo);
    void drawModelTemplateless(shared_ptr<BaseModel> model, shared_ptr<Pipeline> pipeline, void* ubo);

    template <typename TVertex>
    shared_ptr<DynamicModel<TVertex>> getDynamicModel(vector<TVertex>& vertices, vector<uint32_t>& indices);

    template <GenericVertex2D TVertex>
    shared_ptr<DynamicModel<TVertex>> triangulateModel(vector<TVertex>& points);

    BasicUBO getNewUBO();
    BasicUBO getNewUBO(int x, int y, int width, int height, float rotation, vec4 color);

    void log(string txt);
private:
    // Average C++ destruct order error
    vki::CommandPool commandPool;
    vector<vki::CommandBuffer> commandBuffers;

    vector<vki::Semaphore> imageAvailableSemaphores;
    vector<vki::Semaphore> renderFinishedSemaphores;
    vector<vki::Fence> inFlightFences;

    shared_ptr<Shader> basicVertShader;
    shared_ptr<Shader> basicFragShader;
    shared_ptr<Pipeline> basicPipeline;

    shared_ptr<Shader> elipseVertShader;
    shared_ptr<Shader> elipseFragShader;
    shared_ptr<Pipeline> elipsePipeline;
    
    shared_ptr<Model<BasicVertex>> rectangle;
    shared_ptr<Model<BasicVertex>> triangle;

    int dynamicModelsThisFrame = 0;
    vector<vector<shared_ptr<BaseModel>>> dynamicModels;

    QueueFamilyIndices findQueueFamilies(vki::PhysicalDevice device);
    void recreateSwapChain();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    friend class SwapChain;

public:
    // Thanks C++
    unique_ptr<SwapChain> swapChain;
};

template<typename T>
inline void Renderer::createBufferWithStaging(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vki::Buffer& buffer, vki::DeviceMemory& bufferMemory, const vector<T>& data)
{
    auto size = sizeof(T) * data.size();

    vki::Buffer stagingBuffer = { 0 };
    vki::DeviceMemory stagingMemory = { 0 };
    createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingMemory);

    void* ptr = stagingMemory.mapMemory(0, size);
    memcpy(ptr, data.data(), size);
    stagingMemory.unmapMemory();

    createBuffer(size, usage, properties, buffer, bufferMemory);
    copyBuffer(stagingBuffer, buffer, size);
}

template <typename T>
inline void Renderer::drawModel(shared_ptr<BaseModel> model, shared_ptr<Pipeline> pipeline, T ubo)
{
    drawModelTemplateless(model, pipeline, &ubo);
}

template <GenericVertex2D TVertex>
inline shared_ptr<DynamicModel<TVertex>> Renderer::triangulateModel(vector<TVertex>& points)
{
    CDT::Triangulation<float> cdt;
    cdt.insertVertices(points.begin(), points.end(), [](const BasicVertex& p)
                       { return p.pos.x; }, [](const BasicVertex& p)
                       { return p.pos.y; });

    cdt.eraseSuperTriangle();

    vector<uint32_t> indices;

    for (const auto& i : cdt.triangles)
    {
        indices.push_back(i.vertices[0]);
        indices.push_back(i.vertices[1]);
        indices.push_back(i.vertices[2]);
    }

    return getDynamicModel(points, indices);
}

inline void Renderer::drawPolygon(initializer_list<BasicVertex> points, int x, int y, int width, int height, float rotation, vec4 color)
{
    vector<BasicVertex> data = points;
    drawPolygon(data, x, y, width, height, rotation, color);
}
