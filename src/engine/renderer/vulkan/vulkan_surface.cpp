
#include "renderer/vulkan/vulkan_surface.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanSurface::~VulkanSurface()
{
    if (this->v_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(this->v_instance, this->v_surface, nullptr);
    }
}

// ============================================================
// Public Methods
// ============================================================

void VulkanSurface::createWindowSurfaceSDL(VkInstance instance, SDL_Window *window)
{
    if (!window) {
        throw std::runtime_error("Failed Not Found Window!");
    }

    this->v_instance = instance;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &this->v_surface)) {
        throw std::runtime_error("Failed to create SDL3 window surface!");
    }
    LOG_CORE_INFO("[Vulkan] Surface created successfully");
}
