#pragma once

#include "utils.hpp"
#include "Renderer.hpp"

template <typename TVertex>
class VertexBuffer
{
    vki::Buffer handle;
    vki::DeviceMemory memory;
    Renderer* renderer;
public:
    vector<TVertex> vertices;
        
    VertexBuffer(Renderer* renderer, vector<TVertex> vertices);

    void bind(vki::CommandBuffer& cmds);
    void draw(vki::CommandBuffer& cmds);
};

template<typename TVertex>
inline VertexBuffer<TVertex>::VertexBuffer(Renderer* renderer, vector<TVertex> vertices) : renderer(renderer), vertices(vertices), handle({}), memory({})
{
    auto size = sizeof(TVertex) * vertices.size();

    vki::Buffer stagingBuffer = { 0 };
    vki::DeviceMemory stagingMemory = { 0 };
    renderer->createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingMemory);

    void* data = stagingMemory.mapMemory(0, size);
    memcpy(data, vertices.data(), size);
    stagingMemory.unmapMemory();

    renderer->createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, handle, memory);
    renderer->copyBuffer(stagingBuffer, handle, size);
}

template<typename TVertex>
inline void VertexBuffer<TVertex>::bind(vki::CommandBuffer& cmds)
{
    cmds.bindVertexBuffers(0, { handle }, { 0 });
}

template<typename TVertex>
inline void VertexBuffer<TVertex>::draw(vki::CommandBuffer& cmds)
{
    bind(cmds);
    cmds.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
}
