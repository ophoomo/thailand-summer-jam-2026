#include <stdexcept>
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanCommandBuffer::VulkanCommandBuffer(const VkDevice &device, const VkCommandPool &command_pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &this->v_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Command Buffer!");
    }

    LOG_CORE_INFO("[Vulkan] Command Buffer Created");
}
