#ifndef C92D9C13_B6DB_48F7_A074_83606BFB5DA1
#define C92D9C13_B6DB_48F7_A074_83606BFB5DA1

#include <vulkan/vulkan_core.h>

class VulkanRenderPass
{
  public:
    VulkanRenderPass(const VkDevice &device);
    ~VulkanRenderPass();

    [[nodiscard]] VkRenderPass get() const
    {
        return this->v_renderPass;
    };

  private:
    VkRenderPass v_renderPass = VK_NULL_HANDLE;
    VkDevice v_device = VK_NULL_HANDLE;
};

#endif /* C92D9C13_B6DB_48F7_A074_83606BFB5DA1 */
