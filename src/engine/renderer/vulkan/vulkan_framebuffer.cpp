
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanFrameBuffer::VulkanFrameBuffer(const VkDevice &device,
                                     const std::vector<VkImageView> &imageViews,
                                     const VkRenderPass &render_pass, const uint32_t &width,
                                     const uint32_t &height)
{
    this->v_device = device;
    for (const auto view : imageViews) {
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = render_pass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &view;
        fbInfo.width = width;
        fbInfo.height = height;
        fbInfo.layers = 1;

        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(this->v_device, &fbInfo, nullptr, &framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
        this->v_frameBuffers.push_back(framebuffer);
    }
    LOG_CORE_INFO("[Vulkan] FrameBuffers Created");
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
    if (!this->v_frameBuffers.empty()) {
        for (const auto frame : this->v_frameBuffers) {
            vkDestroyFramebuffer(this->v_device, frame, nullptr);
        }
        LOG_CORE_INFO("[Vulkan] FrameBuffers Destroyed");
    }
}
