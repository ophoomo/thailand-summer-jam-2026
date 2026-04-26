#ifndef CF051AAF_9029_444D_B999_5B434652B484
#define CF051AAF_9029_444D_B999_5B434652B484

#include "core/window.h"
#include "renderer/renderer_interface.h"
#include "renderer/text_effect.h"
#include "renderer/vertex.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_command_pool.h"
#include "renderer/vulkan/vulkan_descriptor_set.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_instance.h"
#include "renderer/vulkan/vulkan_logical_device.h"
#include "renderer/vulkan/vulkan_physical_device.h"
#include "renderer/vulkan/vulkan_pipeline_manager.h"
#include "renderer/vulkan/vulkan_render_pass.h"
#include "renderer/vulkan/vulkan_shader_manager.h"
#include "renderer/vulkan/vulkan_surface.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "renderer/vulkan/vulkan_sync_objects.h"
#include "renderer/vulkan/vulkan_texture_manager.h"
#include <memory>

#define MAX_FRAMES_IN_FLIGHT 2

class VulkanRenderer : public RendererInterface
{
  public:
    VulkanRenderer(std::shared_ptr<Window> window);
    ~VulkanRenderer();

    void resize(int width, int height) override;
    void initImGui(void *sdlWindow) override;
    void shutdownImGui() override;

    void BeginFrame() override;
    void EndFrame() override;

    void SubmitVertices(const Vertex2D *verts, uint32_t quadCount, PipelineID pipeline) override;
    TextureHandle createTexture(const uint8_t *pixels, int width, int height,
                                bool srgb = true) override;
    void SubmitTextVertices(const Vertex2D *verts, uint32_t quadCount, TextureHandle texture,
                            const TextEffectGPU &effect) override;
    void SubmitSpriteVertices(const Vertex2D *verts, uint32_t quadCount,
                              TextureHandle texture) override;
    void destroyTexture(TextureHandle handle) override;
    void waitIdle() override;
    void setMsdfUnitRange(float x, float y) override;

  private:
    void buildRenderChain(uint32_t width, uint32_t height);
    void destroyRenderChain();

    struct BatchEntry
    {
        uint32_t vertexOffset;
        uint32_t quadCount;
        PipelineID pipeline;
        TextureHandle texture = INVALID_TEXTURE;
        TextEffectGPU effect{};
    };

    // Core Vulkan objects
    std::unique_ptr<VulkanInstance> v_instance;
    std::unique_ptr<VulkanSurface> v_surface;
    std::unique_ptr<VulkanPhysicalDevice> v_physicalDevice;
    std::shared_ptr<VulkanLogicalDevice> v_logicalDevice;

    // Render chain (recreated on resize)
    std::unique_ptr<VulkanSwapchain> v_swapChain;
    std::unique_ptr<VulkanRenderPass> v_renderPass;
    std::unique_ptr<VulkanFrameBuffer> v_framebuffer;
    std::unique_ptr<VulkanCommandPool> v_cmdPool;
    std::unique_ptr<VulkanCommandBuffer> v_cmdBuffer[MAX_FRAMES_IN_FLIGHT];

    // Buffers
    std::unique_ptr<VulkanBuffer> v_vertexBuffer[MAX_FRAMES_IN_FLIGHT];
    std::unique_ptr<VulkanBuffer> v_indexBuffer;
    std::unique_ptr<VulkanDescriptorSet> v_descriptor;

    // Sync
    std::unique_ptr<VulkanSyncObjects> v_sync[MAX_FRAMES_IN_FLIGHT];

    // Manager
    std::shared_ptr<VulkanShaderManager> m_shaderManager;
    std::unique_ptr<VulkanPipelineManager> m_pipelineManager;
    std::unique_ptr<VulkanTextureManager> m_textureManager;

    std::shared_ptr<Window> m_window;

    std::vector<Vertex2D> m_vertexBatch;
    std::vector<BatchEntry> m_batches;

    int m_currentFrame = 0;
    uint32_t m_imageIndex = 0;
    bool m_imguiReady = false;
    float m_msdfUnitRange[2] = {1.0f, 1.0f};
    bool m_frameSkipped = false;
};

#endif /* CF051AAF_9029_444D_B999_5B434652B484 */
