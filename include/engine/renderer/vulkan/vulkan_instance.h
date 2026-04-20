#ifndef F82C10A3_7E26_42B9_B8FE_3152FD43B08F
#define F82C10A3_7E26_42B9_B8FE_3152FD43B08F

#include <array>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanInstance
{
  public:
    VulkanInstance(const std::string &title, const std::vector<const char *> &extensions);
    ~VulkanInstance();

    [[nodiscard]] VkInstance get() const
    {
        return this->v_instance;
    }

  private:
    static void checkValidationLayerSupport();
    static void checkExtensionSupport(const std::vector<const char *> &extensions);
    void setupDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT &info);

    static VkDebugUtilsMessengerCreateInfoEXT populateDebugCreateInfo();

    VkInstance v_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT v_debugMessenger = VK_NULL_HANDLE;
    static constexpr std::array<const char *, 1> m_validationLayers{"VK_LAYER_KHRONOS_validation"};
#ifndef NDEBUG
    const bool m_enableValidation = true;
#else
    const bool m_enableValidation = false;
#endif
};

#endif /* F82C10A3_7E26_42B9_B8FE_3152FD43B08F */
