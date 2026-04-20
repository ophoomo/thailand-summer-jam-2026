#ifndef BFB22AEF_258D_4298_8A59_95700BE6BEA7
#define BFB22AEF_258D_4298_8A59_95700BE6BEA7

#include <vulkan/vulkan_core.h>

class VulkanCommandBuffer
{
  public:
    VulkanCommandBuffer(const VkDevice &device, const VkCommandPool &command_pool);
    ~VulkanCommandBuffer() = default;

    [[nodiscard]] VkCommandBuffer get() const
    {
        return this->v_commandBuffer;
    };

  private:
    VkCommandBuffer v_commandBuffer = VK_NULL_HANDLE;
};

#endif /* BFB22AEF_258D_4298_8A59_95700BE6BEA7 */
