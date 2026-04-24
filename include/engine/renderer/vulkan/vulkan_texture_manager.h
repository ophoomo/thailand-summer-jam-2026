#ifndef EDB1071C_C01B_483F_999B_0E159446194F
#define EDB1071C_C01B_483F_999B_0E159446194F

#include "renderer/renderer_interface.h"
#include "renderer/vulkan/vulkan_texture.h"
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
class VulkanTextureManager
{
  public:
    VulkanTextureManager() = default;
    ~VulkanTextureManager();

    VulkanTextureManager(const VulkanTextureManager &) = delete;
    VulkanTextureManager &operator=(const VulkanTextureManager &) = delete;

    void init(VkDevice device, VkPhysicalDevice physDevice, VkQueue queue, uint32_t queueFamily,
              VkDescriptorSetLayout textLayout);

    // Upload RGBA8 pixels and return an opaque handle (INVALID_TEXTURE on error).
    [[nodiscard]] TextureHandle upload(const uint8_t *pixels, int width, int height,
                                       bool srgb = true);
    void destroy(TextureHandle handle);
    [[nodiscard]] VkDescriptorSet getDescriptorSet(TextureHandle handle) const;

  private:
    struct Entry
    {
        std::unique_ptr<VulkanTexture> texture;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    VulkanTexture::UploadContext m_ctx{};
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textLayout = VK_NULL_HANDLE;

    std::unordered_map<TextureHandle, Entry> m_entries;
    TextureHandle m_nextHandle = 1; // 0 = INVALID_TEXTURE
};

#endif /* EDB1071C_C01B_483F_999B_0E159446194F */
