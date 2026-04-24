
#include "renderer/vulkan/vulkan_render_pass.h"
#include "utils/logger.h"
#include <stdexcept>

// ============================================================
// Construction / destruction
// ============================================================

VulkanRenderPass::VulkanRenderPass(const VkDevice &device)
{
    this->v_device = device;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(this->v_device, &renderPassInfo, nullptr, &this->v_renderPass) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create RenderPass!");
    }

    LOG_CORE_INFO("[Vulkan] RenderPass Created");
}

VulkanRenderPass::~VulkanRenderPass()
{
    if (this->v_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(this->v_device, this->v_renderPass, nullptr);
        LOG_CORE_INFO("[Vulkan] RenderPass Destroyed");
    }
}
