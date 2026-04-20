#ifndef FCE66294_9206_465E_A34B_7194C6235B17
#define FCE66294_9206_465E_A34B_7194C6235B17

#include <array>
#include <optional>
#include <vulkan/vulkan_core.h>

class VulkanPhysicalDevice
{
  public:
    VulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanPhysicalDevice() = default;

    [[nodiscard]] VkPhysicalDevice get() const
    {
        return this->v_physicalDevice;
    }
    [[nodiscard]] uint32_t graphicsFamily() const
    {
        return this->m_graphicsFamily.value();
    }
    [[nodiscard]] uint32_t presentFamily() const
    {
        return this->m_presentFamily.value();
    }
    [[nodiscard]] bool sameQueueFamilies() const
    {
        return this->m_graphicsFamily == this->m_presentFamily;
    }

  private:
    struct QueueFamilies
    {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
        [[nodiscard]] bool complete() const
        {
            return graphics && present;
        }
    };

    void pick(VkInstance instance, VkSurfaceKHR surface);
    static int rateDevice(VkPhysicalDevice dev);
    static bool checkExtensions(VkPhysicalDevice dev);
    static QueueFamilies findQueueFamilies(VkPhysicalDevice dev, VkSurfaceKHR surface);

    VkPhysicalDevice v_physicalDevice = VK_NULL_HANDLE;
    std::optional<uint32_t> m_graphicsFamily;
    std::optional<uint32_t> m_presentFamily;
    static constexpr std::array<const char *, 1> m_required_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

#endif /* FCE66294_9206_465E_A34B_7194C6235B17 */
