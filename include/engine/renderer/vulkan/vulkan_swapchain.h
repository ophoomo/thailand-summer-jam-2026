#ifndef BC1EF668_216F_43B9_971D_9BADABCCCE97
#define BC1EF668_216F_43B9_971D_9BADABCCCE97

#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanSwapchain
{
  public:
    VulkanSwapchain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface,
                    uint32_t graphicsFamily, uint32_t presentFamily, uint32_t width,
                    uint32_t height);
    ~VulkanSwapchain();

    [[nodiscard]] VkSwapchainKHR get() const
    {
        return this->v_swapChain;
    }
    [[nodiscard]] const std::vector<VkImageView> &imageViews() const
    {
        return this->v_imageViews;
    }
    [[nodiscard]] VkFormat format() const
    {
        return this->v_format;
    }
    [[nodiscard]] VkExtent2D extent() const
    {
        return this->v_extent;
    }
    [[nodiscard]] uint32_t imageCount() const
    {
        return static_cast<uint32_t>(this->v_imageViews.size());
    }

  private:
    static VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice physDevice,
                                                  VkSurfaceKHR surface);
    static VkPresentModeKHR choosePresentMode(VkPhysicalDevice physDevice, VkSurfaceKHR surface);
    static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &caps, uint32_t w, uint32_t h);

    void createImageViews();

    VkDevice v_device = VK_NULL_HANDLE;
    VkSwapchainKHR v_swapChain = VK_NULL_HANDLE;
    std::vector<VkImageView> v_imageViews;
    VkFormat v_format = VK_FORMAT_UNDEFINED;
    VkExtent2D v_extent = {};
};

#endif /* BC1EF668_216F_43B9_971D_9BADABCCCE97 */
