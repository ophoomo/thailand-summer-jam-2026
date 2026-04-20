#ifndef CB795CA8_75BF_4A76_9E83_0F278664DB55
#define CB795CA8_75BF_4A76_9E83_0F278664DB55

#include "renderer/renderer_interface.h"
#include "renderer/vulkan/vulkan_logical_device.h"
#include "renderer/vulkan/vulkan_shader_manager.h"
#include <memory>

// ─── BlendMode ───────────────────────────────────────────────────────────────
//  Controls how the fragment output is composited over the existing framebuffer.
//  Used as a field in PipelineDesc so a single shader pair can serve multiple
//  blend modes without duplicating SPIR-V.
enum class BlendMode : uint8_t {
    None,     // No blending — opaque geometry (depth-write on).
    Alpha,    // src-alpha / one-minus-src-alpha — standard transparent sprite.
    Additive, // src-alpha + dst-colour — fire, glow, particle VFX.
    Multiply, // dst-colour × src-colour — shadows, colour grading overlays.
};

// ─── PipelineDesc ────────────────────────────────────────────────────────────
//  Declarative, data-driven description of a graphics pipeline.
//  PipelineManager compiles this into a VkPipeline on first use.
//  Adding a new visual style only requires a new PipelineDesc entry — no
//  changes to OxRenderer or the draw-call path.
struct PipelineDesc
{
    ShaderID vertShader = INVALID_SHADER_ID;
    ShaderID fragShader = INVALID_SHADER_ID;
    BlendMode blend = BlendMode::Alpha;
};

class VulkanPipelineManager
{
  public:
    VulkanPipelineManager(std::shared_ptr<VulkanLogicalDevice> device,
                          std::shared_ptr<VulkanShaderManager> shaderManager);
    ~VulkanPipelineManager();

    // (Re)compile all built-in pipelines for the given render pass.
    // Safe to call multiple times (destroys old pipelines first).
    void registerAll(VkRenderPass renderPass);

    // Retrieve a compiled pipeline (VK_NULL_HANDLE if not registered).
    [[nodiscard]] VkPipeline getPipeline(PipelineID id) const;
    [[nodiscard]] VkPipelineLayout getLayout() const
    {
        return this->m_layout;
    }
    [[nodiscard]] VkDescriptorSetLayout getDescriptorLayout() const
    {
        return this->m_descriptorLayout;
    }
    [[nodiscard]] VkPipelineLayout getTextLayout() const
    {
        return this->m_textLayout;
    }
    [[nodiscard]] VkDescriptorSetLayout getTextDescriptorSetLayout() const
    {
        return this->m_textDescriptorSetLayout;
    }

  private:
    [[nodiscard]] static VkPipelineColorBlendAttachmentState makeBlendAttachment(BlendMode mode);
    [[nodiscard]] VkPipeline buildPipeline(const PipelineDesc &desc, VkRenderPass rp,
                                           VkPipelineLayout layout = VK_NULL_HANDLE) const;

    void createSharedLayout();
    void destroyPipelines();

    std::shared_ptr<VulkanLogicalDevice> m_device;
    std::shared_ptr<VulkanShaderManager> m_shaderManager;

    std::unordered_map<uint8_t, VkPipeline> m_pipelines;

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;

    VkPipelineLayout m_textLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textDescriptorSetLayout = VK_NULL_HANDLE;
};

#endif /* CB795CA8_75BF_4A76_9E83_0F278664DB55 */
