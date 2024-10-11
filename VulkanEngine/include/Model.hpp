#pragma once

#include "utils.hpp"
#include "Renderer.hpp"

template <typename TVertex>
class Model
{
    vki::Buffer handle;
    vki::DeviceMemory memory;

    vki::Buffer indicesHandle;
    vki::DeviceMemory indicesMemory;

    Renderer* renderer;
public:
    vector<TVertex> vertices;
    vector<uint32_t> indices;

    Model(Renderer* renderer, vector<TVertex> vertices, vector<uint32_t> indices);

    void bind(vki::CommandBuffer& cmds);
    void draw(vki::CommandBuffer& cmds);
};

template<typename TVertex>
inline Model<TVertex>::Model(Renderer* renderer, vector<TVertex> vertices, vector<uint32_t> indices) : renderer(renderer), vertices(vertices), handle({}), memory({}), indices(indices), indicesHandle({}), indicesMemory({})
{
    renderer->createBufferWithStaging(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, handle, memory, vertices);
    renderer->createBufferWithStaging(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indicesHandle, indicesMemory, indices);
}

template<typename TVertex>
inline void Model<TVertex>::bind(vki::CommandBuffer& cmds)
{
    cmds.bindVertexBuffers(0, { handle }, { 0 });
    cmds.bindIndexBuffer(indicesHandle, 0, vk::IndexType::eUint32);
}

template<typename TVertex>
inline void Model<TVertex>::draw(vki::CommandBuffer& cmds)
{
    bind(cmds);
    cmds.drawIndexed(indices.size(), 1, 0, 0, 0);
}
