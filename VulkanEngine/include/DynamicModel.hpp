#pragma once

#include "BaseModel.hpp"

template <typename TVertex>
class DynamicModel : public BaseModel
{
    vki::Buffer handle;
    vki::DeviceMemory memory;

    vki::Buffer indicesHandle;
    vki::DeviceMemory indicesMemory;
public:
    vector<TVertex> vertices;
    vector<uint32_t> indices;

    DynamicModel(Renderer* renderer);
    DynamicModel(Renderer* renderer, vector<TVertex>& vertices, vector<uint32_t>& indices);

    void update(vector<TVertex>& vertices, vector<uint32_t>& indices);
    
    void bind(vki::CommandBuffer& cmds) override;
    void draw(vki::CommandBuffer& cmds) override;
};

template<typename TVertex>
inline DynamicModel<TVertex>::DynamicModel(Renderer* renderer) : handle({}), memory({}), indicesHandle({}), indicesMemory({})
{
    this->renderer = renderer;
}

template<typename TVertex>
inline DynamicModel<TVertex>::DynamicModel(Renderer* renderer, vector<TVertex>& vertices, vector<uint32_t>& indices) : handle({}), memory({}), indicesHandle({}), indicesMemory({})
{
    this->renderer = renderer;
    update(vertices, indices);
}

template<typename TVertex>
inline void DynamicModel<TVertex>::update(vector<TVertex>& vertices, vector<uint32_t>& indices)
{
    if (vertices.size() > this->vertices.size())
    {
        renderer->createBuffer(vertices.size() * sizeof(TVertex), vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, handle, memory);
    }

    if (indices.size() > this->indices.size())
    {
        renderer->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indicesHandle, indicesMemory);
    }

    auto verticesData = memory.mapMemory(0, vertices.size() * sizeof(TVertex));
    memcpy(verticesData, vertices.data(), vertices.size() * sizeof(TVertex));
    memory.unmapMemory();

    auto indicesData = indicesMemory.mapMemory(0, indices.size() * sizeof(uint32_t));
    memcpy(indicesData, indices.data(), indices.size() * sizeof(uint32_t));
    indicesMemory.unmapMemory();
    
    this->vertices = vertices;
    this->indices = indices;
}

template<typename TVertex>
inline void DynamicModel<TVertex>::bind(vki::CommandBuffer& cmds)
{
    cmds.bindVertexBuffers(0, { handle }, { 0 });
    cmds.bindIndexBuffer(indicesHandle, 0, vk::IndexType::eUint32);
}

template<typename TVertex>
inline void DynamicModel<TVertex>::draw(vki::CommandBuffer& cmds)
{
    bind(cmds);
    cmds.drawIndexed(indices.size(), 1, 0, 0, 0);
}

// I love C++ circular dependencies
template <typename TVertex>
shared_ptr<DynamicModel<TVertex>> Renderer::getDynamicModel(vector<TVertex>& vertices, vector<uint32_t>& indices)
{
    shared_ptr<DynamicModel<TVertex>> model;

    if (dynamicModelsThisFrame >= dynamicModels[currentFlightFrame].size())
    {
        model = make_shared<DynamicModel<TVertex>>(this);
        dynamicModels[currentFlightFrame].push_back(model);
    }
    else
    {
        model = static_pointer_cast<DynamicModel<TVertex>>(dynamicModels[currentFlightFrame][dynamicModelsThisFrame]);
    }

    model->update(vertices, indices);

    dynamicModelsThisFrame++;

    return model;
}
