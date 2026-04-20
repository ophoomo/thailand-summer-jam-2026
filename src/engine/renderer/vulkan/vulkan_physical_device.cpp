
#include <algorithm>
#include <stdexcept>

#include "renderer/vulkan/vulkan_physical_device.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    this->pick(instance, surface);
}

bool VulkanPhysicalDevice::checkExtensions(VkPhysicalDevice dev)
{
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, available.data());
    for (const char *req : VulkanPhysicalDevice::m_required_extensions) {
        const bool found = std::ranges::any_of(available, [req](const VkExtensionProperties &e) {
            return strcmp(e.extensionName, req) == 0;
        });
        if (!found)
            return false;
    }
    return true;
}

VulkanPhysicalDevice::QueueFamilies VulkanPhysicalDevice::findQueueFamilies(VkPhysicalDevice dev,
                                                                            VkSurfaceKHR surface)
{
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families.data());

    QueueFamilies qf;
    for (uint32_t i = 0; i < count; ++i) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            qf.graphics = i;

        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present);
        if (present)
            qf.present = i;

        if (qf.complete())
            break;
    }
    return qf;
}

// ============================================================
// Private Methods
// ============================================================

int VulkanPhysicalDevice::rateDevice(VkPhysicalDevice dev)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(dev, &props);

    int score = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ? 1000 : 0;
    uint32_t maxDimension = props.limits.maxImageDimension2D;

    if (maxDimension > std::numeric_limits<int>::max()) {
        score += std::numeric_limits<int>::max();
    } else {
        score += static_cast<int>(maxDimension);
    }
    return score;
}

void VulkanPhysicalDevice::pick(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0)
        throw std::runtime_error("No Vulkan devices found");

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    int bestScore = -1;
    for (auto dev : devices) {
        auto qf = findQueueFamilies(dev, surface);
        if (!qf.complete())
            continue;
        if (!checkExtensions(dev))
            continue;

        int score = rateDevice(dev);
        if (score > bestScore) {
            bestScore = score;
            this->v_physicalDevice = dev;
            this->m_graphicsFamily = qf.graphics;
            this->m_presentFamily = qf.present;
        }
    }

    if (this->v_physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("No suitable GPU found");

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(this->v_physicalDevice, &props);
    LOG_CORE_INFO("[Vulkan] GPU: {}", props.deviceName);
}
