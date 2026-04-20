
#include "renderer/vulkan/vulkan_texture_manager.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanTextureManager::~VulkanTextureManager()
{
    if (m_pool == VK_NULL_HANDLE)
        return;
    m_entries.clear(); // VulkanTexture destructors run here
    vkDestroyDescriptorPool(m_ctx.device, m_pool, nullptr);
}

// ============================================================
// Public Methods
// ============================================================

void VulkanTextureManager::init(VkDevice device, VkPhysicalDevice physDevice, VkQueue queue,
                                uint32_t queueFamily, VkDescriptorSetLayout textLayout)
{
    m_ctx = {device, physDevice, queue, queueFamily};
    m_textLayout = textLayout;

    constexpr uint32_t maxTextures = 256;
    VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxTextures};
    VkDescriptorPoolCreateInfo pi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pi.poolSizeCount = 1;
    pi.pPoolSizes = &poolSize;
    pi.maxSets = maxTextures;
    if (vkCreateDescriptorPool(device, &pi, nullptr, &m_pool) != VK_SUCCESS)
        throw std::runtime_error("[VulkanTextureManager] Failed to create descriptor pool");
}

TextureHandle VulkanTextureManager::upload(const uint8_t *pixels, int width, int height)
{
    if (m_pool == VK_NULL_HANDLE) {
        LOG_CORE_ERROR("[VulkanTextureManager] upload() called before init()");
        return INVALID_TEXTURE;
    }

    Entry entry;

    // Upload pixels to GPU.
    entry.texture = std::make_unique<VulkanTexture>(m_ctx, pixels, width, height);

    // Allocate descriptor set.
    VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ai.descriptorPool = m_pool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &m_textLayout;
    if (vkAllocateDescriptorSets(m_ctx.device, &ai, &entry.descriptorSet) != VK_SUCCESS) {
        LOG_CORE_ERROR("[VulkanTextureManager] Failed to allocate descriptor set");
        return INVALID_TEXTURE;
    }

    // Write the combined image sampler into the descriptor set.
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = entry.texture->sampler();
    imageInfo.imageView = entry.texture->imageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstSet = entry.descriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_ctx.device, 1, &write, 0, nullptr);

    const TextureHandle handle = m_nextHandle++;
    m_entries.emplace(handle, std::move(entry));
    return handle;
}

void VulkanTextureManager::destroy(TextureHandle handle)
{
    auto it = m_entries.find(handle);
    if (it == m_entries.end())
        return;
    if (it->second.descriptorSet != VK_NULL_HANDLE)
        vkFreeDescriptorSets(m_ctx.device, m_pool, 1, &it->second.descriptorSet);
    m_entries.erase(it);
}

VkDescriptorSet VulkanTextureManager::getDescriptorSet(TextureHandle handle) const
{
    auto it = m_entries.find(handle);
    return (it != m_entries.end()) ? it->second.descriptorSet : VK_NULL_HANDLE;
}
