#ifndef BF3FCE16_FF32_4CA5_9597_4C409E04B756
#define BF3FCE16_FF32_4CA5_9597_4C409E04B756

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

using ShaderID = uint32_t;
static constexpr ShaderID INVALID_SHADER_ID = 0;

class VulkanShaderManager
{
  public:
    VulkanShaderManager(VkDevice device);
    ~VulkanShaderManager();

    [[nodiscard]] ShaderID load(const std::string &spvPath);
    [[nodiscard]] VkShaderModule module(ShaderID id) const;

    void unload(ShaderID id);
    void unloadAll();

  private:
    [[nodiscard]] static std::vector<char> readSpv(const std::string &path);

    VkDevice m_device;
    std::unordered_map<std::string, ShaderID> m_pathToId;
    std::unordered_map<ShaderID, VkShaderModule> m_modules;

    ShaderID m_nextId = 1;
};

#endif /* BF3FCE16_FF32_4CA5_9597_4C409E04B756 */
