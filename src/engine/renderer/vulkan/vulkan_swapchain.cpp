
#include <algorithm>
#include <stdexcept>

#include "renderer/vulkan/vulkan_swapchain.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice physDevice, VkSurfaceKHR surface,
                                 uint32_t graphicsFamily, uint32_t presentFamily, uint32_t width,
                                 uint32_t height)
{
    this->v_device = device;
    auto fmt = VulkanSwapchain::chooseSurfaceFormat(physDevice, surface);
    auto mode = VulkanSwapchain::choosePresentMode(physDevice, surface);
    this->v_format = fmt.format;

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &caps);
    this->v_extent = VulkanSwapchain::chooseExtent(caps, width, height);

    uint32_t imgCount = std::clamp(caps.minImageCount + 1, caps.minImageCount,
                                   caps.maxImageCount > 0 ? caps.maxImageCount : UINT32_MAX);

    VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    ci.surface = surface;
    ci.minImageCount = imgCount;
    ci.imageFormat = fmt.format;
    ci.imageColorSpace = fmt.colorSpace;
    ci.imageExtent = this->v_extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = mode;
    ci.clipped = VK_TRUE;

    uint32_t families[] = {graphicsFamily, presentFamily};
    if (graphicsFamily != presentFamily) {
        ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices = families;
    } else {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(device, &ci, nullptr, &this->v_swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");

    createImageViews();
    LOG_CORE_INFO("[Vulkan] Swapchain: {}x{} {}", this->v_extent.width, this->v_extent.height,
                  imgCount);
}

VulkanSwapchain::~VulkanSwapchain()
{
    for (auto iv : this->v_imageViews)
        vkDestroyImageView(this->v_device, iv, nullptr);
    vkDestroySwapchainKHR(this->v_device, this->v_swapChain, nullptr);
}

// ============================================================
// Private Methods
// ============================================================

VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(VkPhysicalDevice pd, VkSurfaceKHR s)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, s, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, s, &count, formats.data());
    for (const auto &f : formats)
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    return formats[0];
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(VkPhysicalDevice pd, VkSurfaceKHR s)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, s, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, s, &count, modes.data());
    for (auto m : modes)
        if (m == VK_PRESENT_MODE_MAILBOX_KHR)
            return m;                // triple-buffer if available
    return VK_PRESENT_MODE_FIFO_KHR; // vsync fallback
}

VkExtent2D VulkanSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR &caps, uint32_t w,
                                         uint32_t h)
{
    if (caps.currentExtent.width != UINT32_MAX)
        return caps.currentExtent;
    return {
        std::clamp(w, caps.minImageExtent.width, caps.maxImageExtent.width),
        std::clamp(h, caps.minImageExtent.height, caps.maxImageExtent.height),
    };
}

void VulkanSwapchain::createImageViews()
{
    uint32_t count;
    vkGetSwapchainImagesKHR(this->v_device, this->v_swapChain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(this->v_device, this->v_swapChain, &count, images.data());

    this->v_imageViews.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        VkImageViewCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        ci.image = images[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = this->v_format;
        ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        if (vkCreateImageView(this->v_device, &ci, nullptr, &this->v_imageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }
}
