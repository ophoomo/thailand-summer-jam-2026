#ifndef B4137C2C_32D5_42FA_BA29_4EC048EDC726
#define B4137C2C_32D5_42FA_BA29_4EC048EDC726

#include <vulkan/vulkan_core.h>

class VulkanSyncObjects
{
  public:
    VulkanSyncObjects(VkDevice device);
    ~VulkanSyncObjects();

    [[nodiscard]] VkSemaphore imageAvailable() const
    {
        return this->v_imageAvailable;
    }
    [[nodiscard]] VkSemaphore renderFinished() const
    {
        return this->v_renderFinished;
    }
    [[nodiscard]] VkFence inFlight() const
    {
        return this->v_inFlight;
    }

  private:
    VkDevice v_device = VK_NULL_HANDLE;
    VkSemaphore v_imageAvailable = VK_NULL_HANDLE;
    VkSemaphore v_renderFinished = VK_NULL_HANDLE;
    VkFence v_inFlight = VK_NULL_HANDLE;
};

#endif /* B4137C2C_32D5_42FA_BA29_4EC048EDC726 */
