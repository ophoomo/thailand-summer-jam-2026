
#include <array>
#include <stdexcept>

#include "renderer/vulkan/vulkan_logical_device.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanLogicalDevice::VulkanLogicalDevice(VkPhysicalDevice physicalDevice,
                                         uint32_t graphicsQueueIndex)
{
    float priority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsQueueIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    constexpr std::array<const char *, 1> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &this->v_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    LOG_CORE_INFO("[Vulkan] Logical Device Created");

    vkGetDeviceQueue(this->v_device, graphicsQueueIndex, 0, &this->v_queue);
    LOG_CORE_INFO("[Vulkan] Queue Created");
}

VulkanLogicalDevice::~VulkanLogicalDevice()
{
    vkDestroyDevice(this->v_device, nullptr);
}
