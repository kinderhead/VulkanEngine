#include "Pipeline.hpp"

#include "Renderer.hpp"

Pipeline::Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders, VertexDefinition vertexDef, vk::DeviceSize uboSize) : renderer(renderer), layout({}), handle({}), descriptorLayout({}), uboSize(uboSize), descriptorPool({}), descriptorSets({})
{
    // Make UBO
    auto uboLayoutBinding = vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

    auto layoutInfo = vk::DescriptorSetLayoutCreateInfo({}, 1, &uboLayoutBinding);
    descriptorLayout = renderer->device.createDescriptorSetLayout(layoutInfo);

    for (int i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers.push_back({0});
        uniformBuffersMemory.push_back({0});

        renderer->createBuffer(uboSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i]);
        uniformBuffersMapped.push_back(uniformBuffersMemory[i].mapMemory(0, uboSize));
    }

    auto poolSize = vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, renderer->MAX_FRAMES_IN_FLIGHT);
    auto poolInfo = vk::DescriptorPoolCreateInfo({}, renderer->MAX_FRAMES_IN_FLIGHT, 1, &poolSize);

    descriptorPool = renderer->device.createDescriptorPool(poolInfo);

    vector<vk::DescriptorSetLayout> layouts(renderer->MAX_FRAMES_IN_FLIGHT, descriptorLayout);
    auto allocInfo = vk::DescriptorSetAllocateInfo(descriptorPool, renderer->MAX_FRAMES_IN_FLIGHT, layouts.data());
    descriptorSets = vki::DescriptorSets(renderer->device, allocInfo);

    for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto writeSet = vk::WriteDescriptorSet(descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, array<vk::DescriptorBufferInfo, 1>{ vk::DescriptorBufferInfo(uniformBuffers[i], 0, uboSize) }.data());
        renderer->device.updateDescriptorSets({ 1, &writeSet }, { });
    }
    
    // Do pipeline stuff
    renderPass = make_shared<RenderPass>(renderer);

    vector<vk::PipelineShaderStageCreateInfo> stages;

    for (const auto& i : shaders)
    {
        stages.push_back(i->getStageInfo());
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexDef.binding;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDef.attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexDef.attributes.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) renderer->swapChain->extent.width;
    viewport.height = (float) renderer->swapChain->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = renderer->swapChain->extent;

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &*descriptorLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    try
    {
        layout = renderer->device.createPipelineLayout(pipelineLayoutInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error creating graphics layout");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass->handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    try
    {
        handle = renderer->device.createGraphicsPipeline(nullptr, pipelineInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error creating graphics pipeline");
    }
}

void Pipeline::setUBO(void* ubo)
{
    memcpy(uniformBuffersMapped[renderer->currentFlightFrame], ubo, uboSize);
}

void Pipeline::bind(vki::CommandBuffer& cmds)
{
    cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, {descriptorSets[renderer->currentFlightFrame]}, {});
}
