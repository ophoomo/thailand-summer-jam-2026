#ifndef EB5B7CB6_180E_41AC_B832_2D8E9970BE5B
#define EB5B7CB6_180E_41AC_B832_2D8E9970BE5B

#include <vulkan/vulkan_core.h>

class VulkanBuffer
{
  public:
    VulkanBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size,
                 VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
    ~VulkanBuffer();

    void upload(const void *data, VkDeviceSize bytes) const;

    [[nodiscard]] VkBuffer handle() const
    {
        return this->v_buffer;
    }
    [[nodiscard]] VkDeviceSize size() const
    {
        return this->v_size;
    }
    [[nodiscard]] bool empty() const
    {
        return this->v_size == 0;
    }

  private:
    static uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t filter,
                                   VkMemoryPropertyFlags flags);

    VkDevice v_device = VK_NULL_HANDLE;
    VkBuffer v_buffer = VK_NULL_HANDLE;
    VkDeviceMemory v_memory = VK_NULL_HANDLE;
    VkDeviceSize v_size = 0;

    void *v_mapped = nullptr; // non-null for persistently-mapped host-visible buffers
};

#endif /* EB5B7CB6_180E_41AC_B832_2D8E9970BE5B */
