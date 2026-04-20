#ifndef CB058B80_BDDE_4E0A_85F2_EBDAEBF20B64
#define CB058B80_BDDE_4E0A_85F2_EBDAEBF20B64

#include "SDL3/SDL_video.h"
#include <vulkan/vulkan_core.h>

class VulkanSurface
{
  public:
    VulkanSurface() = default;
    ~VulkanSurface();

    [[nodiscard]] VkSurfaceKHR get() const
    {
        return this->v_surface;
    }
    void createWindowSurfaceSDL(VkInstance instance, SDL_Window *window);

  private:
    VkInstance v_instance = VK_NULL_HANDLE;
    VkSurfaceKHR v_surface = VK_NULL_HANDLE;
};

#endif /* CB058B80_BDDE_4E0A_85F2_EBDAEBF20B64 */
