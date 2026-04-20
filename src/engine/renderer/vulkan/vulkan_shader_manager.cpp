
#include "renderer/vulkan/vulkan_shader_manager.h"
#include "utils/resource_path.h"
#include <cassert>
#include <fstream>
#include <stdexcept>

// ============================================================
// Construction / destruction
// ============================================================

VulkanShaderManager::VulkanShaderManager(VkDevice device)
{
    this->m_device = device;
}

VulkanShaderManager::~VulkanShaderManager()
{
    this->unloadAll();
}

// ============================================================
// Public Methods
// ============================================================

ShaderID VulkanShaderManager::load(const std::string &spvPath)
{
    // Return cached ID to avoid redundant vkCreateShaderModule calls.
    if (auto it = m_pathToId.find(spvPath); it != m_pathToId.end())
        return it->second;

    const auto code = readSpv(spvPath);

    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule mod = VK_NULL_HANDLE;
    if (vkCreateShaderModule(m_device, &ci, nullptr, &mod) != VK_SUCCESS)
        throw std::runtime_error("VulkanShaderManager: vkCreateShaderModule failed: " + spvPath);

    const ShaderID id = m_nextId++;
    m_pathToId[spvPath] = id;
    m_modules[id] = mod;
    return id;
}

VkShaderModule VulkanShaderManager::module(ShaderID id) const
{
    if (auto it = m_modules.find(id); it != m_modules.end())
        return it->second;

    assert(false && "VulkanShaderManager::module — invalid ShaderID");
    return VK_NULL_HANDLE;
}

void VulkanShaderManager::unload(ShaderID id)
{
    auto it = m_modules.find(id);
    if (it == m_modules.end())
        return;

    vkDestroyShaderModule(m_device, it->second, nullptr);
    m_modules.erase(it);

    for (auto pit = m_pathToId.begin(); pit != m_pathToId.end(); ++pit) {
        if (pit->second == id) {
            m_pathToId.erase(pit);
            break;
        }
    }
}

void VulkanShaderManager::unloadAll()
{
    for (auto &[id, mod] : m_modules)
        vkDestroyShaderModule(m_device, mod, nullptr);
    m_modules.clear();
    m_pathToId.clear();
}

// ============================================================
// Private Methods
// ============================================================

std::vector<char> VulkanShaderManager::readSpv(const std::string &path)
{
    const std::string fullPath = ResourcePath::resolve(path);
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("VulkanShaderManager: cannot open SPIR-V file: " + fullPath);

    const auto size = static_cast<std::streamsize>(file.tellg());
    std::vector<char> buf(static_cast<size_t>(size));
    file.seekg(0);
    file.read(buf.data(), size);
    return buf;
}
