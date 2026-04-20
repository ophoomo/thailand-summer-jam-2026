#ifndef ECFF859E_686C_4FE1_94DF_A84A2E964D38
#define ECFF859E_686C_4FE1_94DF_A84A2E964D38

#include <vulkan/vulkan_core.h>

class VulkanDescriptorSet
{
  public:
    VulkanDescriptorSet(VkDevice device, VkBuffer uboBuffer, VkDeviceSize uboSize);
    ~VulkanDescriptorSet();

    [[nodiscard]] VkDescriptorSetLayout layout() const
    {
        return this->m_layout;
    }
    [[nodiscard]] VkDescriptorSet set() const
    {
        return this->m_set;
    }

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_set = VK_NULL_HANDLE;
};

#endif /* ECFF859E_686C_4FE1_94DF_A84A2E964D38 */
