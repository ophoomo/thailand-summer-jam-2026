
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "renderer/vulkan/vulkan_buffer.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size,
                           VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{
    this->v_device = device;
    this->v_size = size;

    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bi, nullptr, &this->v_buffer) != VK_SUCCESS) {

        auto error = std::format("[Vulkan] Failed to create buffer");
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, this->v_buffer, &req);

    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = VulkanBuffer::findMemoryType(physDevice, req.memoryTypeBits, props);

    if (vkAllocateMemory(device, &ai, nullptr, &this->v_memory) != VK_SUCCESS) {

        auto error = std::format("[Vulkan] Failed to allocate buffer memory");
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }

    vkBindBufferMemory(device, this->v_buffer, this->v_memory, 0);

    if (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vkMapMemory(this->v_device, this->v_memory, 0, VK_WHOLE_SIZE, 0, &this->v_mapped);
    }
}

VulkanBuffer::~VulkanBuffer()
{
    if (v_mapped) {
        vkUnmapMemory(this->v_device, this->v_memory);
    }
    vkDestroyBuffer(this->v_device, this->v_buffer, nullptr);
    vkFreeMemory(this->v_device, this->v_memory, nullptr);
}

// ============================================================
// Public Methods
// ============================================================

void VulkanBuffer::upload(const void *data, VkDeviceSize bytes) const
{
    if (this->v_mapped) {
        std::memcpy(this->v_mapped, data, bytes);
    } else {
        void *mapped;
        vkMapMemory(this->v_device, this->v_memory, 0, bytes, 0, &mapped);
        std::memcpy(mapped, data, bytes);
        vkUnmapMemory(this->v_device, this->v_memory);
    }
}

// ============================================================
// Private Methods
// ============================================================

uint32_t VulkanBuffer::findMemoryType(VkPhysicalDevice physDevice, uint32_t filter,
                                      VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if ((filter & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    auto error = std::format("[Vulkan] No suitable memory type found");
    LOG_CORE_ERROR(error);
    throw std::runtime_error(error);
}
