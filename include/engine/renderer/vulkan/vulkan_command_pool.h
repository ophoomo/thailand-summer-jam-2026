#ifndef D59F98E0_19BF_4012_835C_869271E14D15
#define D59F98E0_19BF_4012_835C_869271E14D15

#include <vulkan/vulkan_core.h>

class VulkanCommandPool
{
  public:
    VulkanCommandPool(const VkDevice &device, const uint32_t &graphicsQueueFamily);
    ~VulkanCommandPool();

    [[nodiscard]] VkCommandPool get() const
    {
        return this->v_commandPool;
    };

  private:
    VkDevice v_device = VK_NULL_HANDLE;
    VkCommandPool v_commandPool = VK_NULL_HANDLE;
};

#endif /* D59F98E0_19BF_4012_835C_869271E14D15 */
