#ifndef D6020124_7473_4060_B2B6_26E44B591F64
#define D6020124_7473_4060_B2B6_26E44B591F64

#include <vulkan/vulkan_core.h>

class VulkanLogicalDevice
{
  public:
    VulkanLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueIndex);
    ~VulkanLogicalDevice();

    [[nodiscard]] VkDevice get() const
    {
        return this->v_device;
    }
    [[nodiscard]] VkQueue getQueue() const
    {
        return this->v_queue;
    }

  private:
    VkDevice v_device = VK_NULL_HANDLE;
    VkQueue v_queue = VK_NULL_HANDLE;
};

#endif /* D6020124_7473_4060_B2B6_26E44B591F64 */
