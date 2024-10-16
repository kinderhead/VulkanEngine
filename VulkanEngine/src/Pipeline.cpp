#include "Pipeline.hpp"

#include "Renderer.hpp"

Pipeline::Pipeline(Renderer* renderer, vector<shared_ptr<Shader>> shaders, VertexDefinition vertexDef, vk::DeviceSize uboSize) : renderer(renderer), layout({}), handle({}), descriptorLayout({}), uboSize(uboSize)
{
    // Make UBO
    auto uboLayoutBinding = vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

    auto layoutInfo = vk::DescriptorSetLayoutCreateInfo({}, 1, &uboLayoutBinding);
    descriptorLayout = renderer->device.createDescriptorSetLayout(layoutInfo);
    
    // Do pipeline stuff
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

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) renderer->swapChain->extent.width;
    viewport.height = (float) renderer->swapChain->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

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
    rasterizer.frontFace = vk::FrontFace::eClockwise;
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

    vector<vk::DynamicState> states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    auto dynamicState = vk::PipelineDynamicStateCreateInfo({}, {states});

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
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderer->renderPass->handle;
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

void Pipeline::beginFrame()
{
    uniformSetsThisFrame = 0;
}

void Pipeline::bind(vki::CommandBuffer& cmds)
{
    cmds.bindPipeline(vk::PipelineBindPoint::eGraphics, handle);

    cmds.setViewport(0, {viewport});
    cmds.setScissor(0, {scissor});
}

shared_ptr<UniformSet> Pipeline::getUniformSet()
{
    if (uniformSetsThisFrame >= uniforms.size())
    {
        auto set = make_shared<UniformSet>(this, descriptorLayout, uboSize);
        uniforms.push_back(set);
    }

    return uniforms[uniformSetsThisFrame++];
}
