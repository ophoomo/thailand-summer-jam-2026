#ifndef D961242B_2CD3_4E6C_8AAE_CE0CAA9699F9
#define D961242B_2CD3_4E6C_8AAE_CE0CAA9699F9

#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanFrameBuffer
{
  public:
    VulkanFrameBuffer(const VkDevice &device, const std::vector<VkImageView> &imageViews,
                      const VkRenderPass &render_pass, const uint32_t &width,
                      const uint32_t &height);
    ~VulkanFrameBuffer();

    [[nodiscard]] std::vector<VkFramebuffer> get() const
    {
        return this->v_frameBuffers;
    }

  private:
    VkDevice v_device = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> v_frameBuffers;
};

#endif /* D961242B_2CD3_4E6C_8AAE_CE0CAA9699F9 */
