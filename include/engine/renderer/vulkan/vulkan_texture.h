#ifndef F5D66A30_A561_4708_B62D_FDE3235CFC2C
#define F5D66A30_A561_4708_B62D_FDE3235CFC2C

#include <vulkan/vulkan_core.h>

class VulkanTexture
{
  public:
    struct UploadContext
    {
        VkDevice device;
        VkPhysicalDevice physDevice;
        VkQueue queue;
        uint32_t queueFamily;
    };

    VulkanTexture(const UploadContext &ctx, const uint8_t *pixels, int width, int height);
    ~VulkanTexture();

    VulkanTexture(const VulkanTexture &) = delete;
    VulkanTexture &operator=(const VulkanTexture &) = delete;

    [[nodiscard]] VkImageView imageView() const
    {
        return m_imageView;
    }
    [[nodiscard]] VkSampler sampler() const
    {
        return m_sampler;
    }

  private:
    static uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t filter,
                                   VkMemoryPropertyFlags flags);

    static void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout from,
                                      VkImageLayout to);

    VkDevice m_device = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};

#endif /* F5D66A30_A561_4708_B62D_FDE3235CFC2C */
