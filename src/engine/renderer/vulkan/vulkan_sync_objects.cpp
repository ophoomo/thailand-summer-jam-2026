
#include <stdexcept>

#include "renderer/vulkan/vulkan_sync_objects.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanSyncObjects::VulkanSyncObjects(VkDevice device)
{
    this->v_device = device;
    VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &si, nullptr, &this->v_imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(device, &si, nullptr, &this->v_renderFinished) != VK_SUCCESS ||
        vkCreateFence(device, &fi, nullptr, &this->v_inFlight) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sync objects");

    LOG_CORE_INFO("[Vulkan] Sync objects created");
}

VulkanSyncObjects::~VulkanSyncObjects()
{
    vkDestroySemaphore(this->v_device, this->v_imageAvailable, nullptr);
    vkDestroySemaphore(this->v_device, this->v_renderFinished, nullptr);
    vkDestroyFence(this->v_device, this->v_inFlight, nullptr);
}
