
#include <stdexcept>

#include "renderer/vulkan/vulkan_descriptor_set.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkBuffer uboBuffer, VkDeviceSize uboSize)
{
    this->m_device = device;

    // Layout: binding 0 = UBO, vertex stage
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &this->m_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");

    // Pool
    VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &this->m_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool");

    // Allocate
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = this->m_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &this->m_layout;
    if (vkAllocateDescriptorSets(device, &allocInfo, &this->m_set) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor set");

    // Write
    VkDescriptorBufferInfo bufInfo{};
    bufInfo.buffer = uboBuffer;
    bufInfo.offset = 0;
    bufInfo.range = uboSize;

    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstSet = m_set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &bufInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

    LOG_CORE_INFO("[Vulkan] DescriptorSet created");
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    vkDestroyDescriptorPool(this->m_device, this->m_pool, nullptr);
    vkDestroyDescriptorSetLayout(this->m_device, this->m_layout, nullptr);
}
