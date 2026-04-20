
#include "renderer/vulkan/vulkan_command_pool.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanCommandPool::VulkanCommandPool(const VkDevice &device, const uint32_t &graphicsQueueFamily)
{
    this->v_device = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamily;

    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(this->v_device, &poolInfo, nullptr, &this->v_commandPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create Command Pool!");
    }

    LOG_CORE_INFO("[Vulkan] Command Pool Created");
}

VulkanCommandPool::~VulkanCommandPool()
{
    if (this->v_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(this->v_device, this->v_commandPool, nullptr);
        LOG_CORE_INFO("[Vulkan] Command Pool Destroyed");
    }
}
