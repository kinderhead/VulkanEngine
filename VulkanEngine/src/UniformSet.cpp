#include "UniformSet.hpp"

#include "Pipeline.hpp"
#include "Renderer.hpp"

UniformSet::UniformSet(Pipeline* pipeline, vki::DescriptorSetLayout& descriptorLayout, vk::DeviceSize uboSize) : pipeline(pipeline), uboSize(uboSize), descriptorPool({})
{
    auto poolSize = vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, pipeline->renderer->MAX_FRAMES_IN_FLIGHT);
    auto poolInfo = vk::DescriptorPoolCreateInfo({}, pipeline->renderer->MAX_FRAMES_IN_FLIGHT, 1, &poolSize);

    descriptorPool = pipeline->renderer->device.createDescriptorPool(poolInfo);

    for (int i = 0; i < pipeline->renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers.push_back({0});
        uniformBuffersMemory.push_back({0});

        pipeline->renderer->createBuffer(uboSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i]);
        uniformBuffersMapped.push_back(uniformBuffersMemory[i].mapMemory(0, uboSize));
    }

    vector<vk::DescriptorSetLayout> layouts(pipeline->renderer->MAX_FRAMES_IN_FLIGHT, descriptorLayout);
    auto allocInfo = vk::DescriptorSetAllocateInfo(descriptorPool, pipeline->renderer->MAX_FRAMES_IN_FLIGHT, layouts.data());
    descriptorSets = vki::DescriptorSets(pipeline->renderer->device, allocInfo);

    for (size_t i = 0; i < pipeline->renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto writeSet = vk::WriteDescriptorSet(descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, array<vk::DescriptorBufferInfo, 1>{ vk::DescriptorBufferInfo(uniformBuffers[i], 0, uboSize) }.data());
        pipeline->renderer->device.updateDescriptorSets({ 1, &writeSet }, { });
    }
}

void UniformSet::setUBO(void* ubo)
{
    memcpy(uniformBuffersMapped[pipeline->renderer->currentFlightFrame], ubo, uboSize);
}

void UniformSet::bind(vki::CommandBuffer& cmds)
{
    cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->layout, 0, {descriptorSets[pipeline->renderer->currentFlightFrame]}, {});
}
