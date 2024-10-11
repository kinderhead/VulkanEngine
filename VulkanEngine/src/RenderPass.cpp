#include "RenderPass.hpp"
#include "Renderer.hpp"



RenderPass::RenderPass(Renderer* renderer) : renderer(renderer), handle({})
{
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = renderer->swapChain->imageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    try
    {
        handle = renderer->device.createRenderPass(renderPassInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("Error creating render pass");
    }
}

vk::RenderPassBeginInfo RenderPass::getBeginInfo(vki::Framebuffer& framebuffer, vk::ClearValue* clearColor)
{
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = handle;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D { 0, 0 };
    renderPassInfo.renderArea.extent = renderer->swapChain->extent;

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearColor;
    return renderPassInfo;
}
