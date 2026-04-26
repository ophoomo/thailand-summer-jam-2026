
#include "renderer/vulkan/vulkan_renderer.h"
#include "core/window.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "renderer/draw_command_buffer.h"
#include "renderer/vertex.h"
#include "renderer/vulkan/vulkan_instance.h"
#include "renderer/vulkan/vulkan_logical_device.h"
#include "renderer/vulkan/vulkan_physical_device.h"
#include "renderer/vulkan/vulkan_pipeline_manager.h"
#include "renderer/vulkan/vulkan_shader_manager.h"
#include "renderer/vulkan/vulkan_surface.h"
#include "renderer/vulkan/vulkan_texture_manager.h"
#include <cstdint>
#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// ============================================================
// Construction / destruction
// ============================================================

VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window)
{
    this->m_window = window;
    this->v_instance =
        std::make_unique<VulkanInstance>(window->getTitle(), window->GetInstanceExtensions());

    this->v_surface = std::make_unique<VulkanSurface>();
    this->v_surface->createWindowSurfaceSDL(this->v_instance->get(), window->get());

    this->v_physicalDevice =
        std::make_unique<VulkanPhysicalDevice>(this->v_instance->get(), this->v_surface->get());

    this->v_logicalDevice = std::make_shared<VulkanLogicalDevice>(
        this->v_physicalDevice->get(), this->v_physicalDevice->graphicsFamily());

    this->m_shaderManager = std::make_shared<VulkanShaderManager>(this->v_logicalDevice->get());
    this->m_pipelineManager =
        std::make_unique<VulkanPipelineManager>(this->v_logicalDevice, this->m_shaderManager);
    this->m_textureManager = std::make_unique<VulkanTextureManager>();
    this->m_textureManager->init(this->v_logicalDevice->get(), this->v_physicalDevice->get(),
                                 this->v_logicalDevice->getQueue(),
                                 this->v_physicalDevice->graphicsFamily(),
                                 this->m_pipelineManager->getTextDescriptorSetLayout());

    this->buildRenderChain(window->getWidth(), window->getHeight());

    this->m_vertexBatch.reserve(MAX_VERTICES);
    this->m_batches.reserve(static_cast<uint8_t>(PipelineID::Count));
}

VulkanRenderer::~VulkanRenderer()
{
    vkDeviceWaitIdle(this->v_logicalDevice->get());
    this->shutdownImGui();
    this->destroyRenderChain();
    this->v_indexBuffer.reset();
}

// ============================================================
// Public Methods
// ============================================================

void VulkanRenderer::BeginFrame()
{
    VkDevice device = this->v_logicalDevice->get();
    VkCommandBuffer cmdBuffer = this->v_cmdBuffer[this->m_currentFrame]->get();
    VkFence renderFence = this->v_sync[this->m_currentFrame]->inFlight();

    m_frameSkipped = false;

    if (this->m_window->isMinimized()) {
        m_frameSkipped = true;
        return;
    }

    VkResult result = vkAcquireNextImageKHR(device, this->v_swapChain->get(), UINT64_MAX,
                          this->v_sync[this->m_currentFrame]->imageAvailable(), VK_NULL_HANDLE,
                          &this->m_imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        this->resize(this->m_window->getWidth(), this->m_window->getHeight());
        m_frameSkipped = true;
        return;
    }

    vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &renderFence);
    vkResetCommandBuffer(cmdBuffer, 0);

    this->m_vertexBatch.clear();
    this->m_batches.clear();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->v_renderPass->get();
    renderPassInfo.framebuffer = this->v_framebuffer->get()[this->m_imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->v_swapChain->extent();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = static_cast<float>(this->v_swapChain->extent().width);
    viewport.height = static_cast<float>(this->v_swapChain->extent().height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = this->v_swapChain->extent();
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    vkCmdBindIndexBuffer(cmdBuffer, this->v_indexBuffer->handle(), 0, VK_INDEX_TYPE_UINT16);

    const float screenSize[2] = {static_cast<float>(this->v_swapChain->extent().width),
                                 static_cast<float>(this->v_swapChain->extent().height)};

    vkCmdPushConstants(cmdBuffer, this->m_pipelineManager->getLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(screenSize), screenSize);

    vkCmdPushConstants(cmdBuffer, this->m_pipelineManager->getTextLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(screenSize), screenSize);

    vkCmdPushConstants(cmdBuffer, this->m_pipelineManager->getTextLayout(),
                       VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(screenSize), sizeof(m_msdfUnitRange),
                       m_msdfUnitRange);
}

void VulkanRenderer::EndFrame()
{
    if (m_frameSkipped) {
        return;
    }
    if (this->m_batches.empty()) {
        return;
    }

    VkCommandBuffer cmd = this->v_cmdBuffer[this->m_currentFrame]->get();
    VkFence renderFence = this->v_sync[this->m_currentFrame]->inFlight();
    VkBuffer vb = v_vertexBuffer[this->m_currentFrame]->handle();

    v_vertexBuffer[this->m_currentFrame]->upload(this->m_vertexBatch.data(),
                                                 this->m_vertexBatch.size() * sizeof(Vertex2D));

    PipelineID lastPipeline = PipelineID::Count;
    TextureHandle lastTexture = INVALID_TEXTURE;
    for (const BatchEntry &batch : this->m_batches) {
        if (batch.pipeline != lastPipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              this->m_pipelineManager->getPipeline(batch.pipeline));
            lastPipeline = batch.pipeline;
        }

        if ((batch.pipeline == PipelineID::Text || batch.pipeline == PipelineID::Sprite) &&
            batch.texture != lastTexture) {
            VkDescriptorSet ds = this->m_textureManager->getDescriptorSet(batch.texture);
            if (ds != VK_NULL_HANDLE) {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        this->m_pipelineManager->getTextLayout(), 0, 1, &ds, 0,
                                        nullptr);
            }
            lastTexture = batch.texture;
        }

        if (batch.pipeline == PipelineID::Text) {
            constexpr uint32_t effectOffset = sizeof(float) * 4; // after screenSize+unitRange
            vkCmdPushConstants(cmd, this->m_pipelineManager->getTextLayout(),
                               VK_SHADER_STAGE_FRAGMENT_BIT, effectOffset, sizeof(TextEffectGPU),
                               &batch.effect);
        }

        const VkDeviceSize byteOffset = batch.vertexOffset * 4 * sizeof(Vertex2D);
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &byteOffset);
        vkCmdDrawIndexed(cmd, batch.quadCount * 6, 1, 0, 0, 0);
    }

    if (this->m_imguiReady && ImGui::GetDrawData()) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSemaphore waitSems[] = {this->v_sync[m_currentFrame]->imageAvailable()};
    VkSemaphore signalSems[] = {this->v_sync[m_currentFrame]->renderFinished()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = waitSems;
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = signalSems;
    vkQueueSubmit(this->v_logicalDevice->getQueue(), 1, &submit, renderFence);

    VkSwapchainKHR sc = this->v_swapChain->get();
    VkSemaphore waitSem[] = {this->v_sync[this->m_currentFrame]->renderFinished()};
    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = waitSem;
    present.swapchainCount = 1;
    present.pSwapchains = &sc;
    present.pImageIndices = &this->m_imageIndex;
    VkResult presentResult = vkQueuePresentKHR(this->v_logicalDevice->getQueue(), &present);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        this->resize(this->m_window->getWidth(), this->m_window->getHeight());
    }

    this->m_currentFrame = (this->m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::resize(int width, int height)
{
    vkDeviceWaitIdle(this->v_logicalDevice->get());
    this->destroyRenderChain();
    this->buildRenderChain(width, height);
    if (this->m_imguiReady) {
        ImGui_ImplVulkan_SetMinImageCount(v_swapChain->imageCount());
    }
}

void VulkanRenderer::SubmitVertices(const Vertex2D *verts, uint32_t quadCount, PipelineID pipeline)
{
    if (quadCount == 0)
        return;

    const uint32_t vertexOffset = static_cast<uint32_t>(this->m_vertexBatch.size()) / 4;
    this->m_vertexBatch.insert(this->m_vertexBatch.end(), verts, verts + quadCount * 4);
    this->m_batches.push_back({vertexOffset, quadCount, pipeline, INVALID_TEXTURE});
}

void VulkanRenderer::SubmitTextVertices(const Vertex2D *verts, uint32_t quadCount,
                                        TextureHandle texture, const TextEffectGPU &effect)
{
    if (quadCount == 0 || texture == INVALID_TEXTURE)
        return;

    const uint32_t vertexOffset = static_cast<uint32_t>(this->m_vertexBatch.size()) / 4;
    this->m_vertexBatch.insert(this->m_vertexBatch.end(), verts, verts + quadCount * 4);
    this->m_batches.push_back({vertexOffset, quadCount, PipelineID::Text, texture, effect});
}

void VulkanRenderer::SubmitSpriteVertices(const Vertex2D *verts, uint32_t quadCount,
                                          TextureHandle texture)
{
    if (quadCount == 0 || texture == INVALID_TEXTURE)
        return;

    const uint32_t vertexOffset = static_cast<uint32_t>(this->m_vertexBatch.size()) / 4;
    this->m_vertexBatch.insert(this->m_vertexBatch.end(), verts, verts + quadCount * 4);
    this->m_batches.push_back({vertexOffset, quadCount, PipelineID::Sprite, texture});
}

TextureHandle VulkanRenderer::createTexture(const uint8_t *pixels, int width, int height, bool srgb)
{
    return this->m_textureManager->upload(pixels, width, height, srgb);
}

void VulkanRenderer::destroyTexture(TextureHandle handle)
{
    this->m_textureManager->destroy(handle);
}

void VulkanRenderer::waitIdle()
{
    vkDeviceWaitIdle(this->v_logicalDevice->get());
}

void VulkanRenderer::setMsdfUnitRange(float x, float y)
{
    m_msdfUnitRange[0] = x;
    m_msdfUnitRange[1] = y;
}

void VulkanRenderer::initImGui(void *sdlWindow)
{
    ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window *>(sdlWindow));

    uint32_t apiVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&apiVersion);

    ImGui_ImplVulkan_InitInfo info{};
    info.ApiVersion = apiVersion;
    info.Instance = v_instance->get();
    info.PhysicalDevice = v_physicalDevice->get();
    info.Device = v_logicalDevice->get();
    info.QueueFamily = v_physicalDevice->graphicsFamily();
    info.Queue = v_logicalDevice->getQueue();
    info.PipelineInfoMain.RenderPass = v_renderPass->get();
    info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    info.ImageCount = v_swapChain->imageCount();
    info.DescriptorPoolSize = 1000;

    ImGui_ImplVulkan_Init(&info);
    this->m_imguiReady = true;
}

void VulkanRenderer::shutdownImGui()
{
    if (!this->m_imguiReady)
        return;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    this->m_imguiReady = false;
}

// ============================================================
// Private Methods
// ============================================================

void VulkanRenderer::destroyRenderChain()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        this->v_sync[i].reset();
        this->v_cmdBuffer[i].reset();
        v_vertexBuffer[i].reset();
    }
    this->v_cmdPool.reset();
    this->v_framebuffer.reset();
    this->v_renderPass.reset();
    this->v_swapChain.reset();
}

void VulkanRenderer::buildRenderChain(uint32_t w, uint32_t h)
{
    this->v_swapChain = std::make_unique<VulkanSwapchain>(
        this->v_logicalDevice->get(), this->v_physicalDevice->get(), this->v_surface->get(),
        this->v_physicalDevice->graphicsFamily(), this->v_physicalDevice->presentFamily(), w, h);

    this->v_renderPass = std::make_unique<VulkanRenderPass>(this->v_logicalDevice->get());

    this->v_framebuffer = std::make_unique<VulkanFrameBuffer>(
        this->v_logicalDevice->get(), this->v_swapChain->imageViews(), this->v_renderPass->get(),
        this->v_swapChain->extent().width, this->v_swapChain->extent().height);

    this->v_cmdPool = std::make_unique<VulkanCommandPool>(this->v_logicalDevice->get(),
                                                          this->v_physicalDevice->graphicsFamily());

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        this->v_cmdBuffer[i] = std::make_unique<VulkanCommandBuffer>(this->v_logicalDevice->get(),
                                                                     this->v_cmdPool->get());
        this->v_sync[i] = std::make_unique<VulkanSyncObjects>(this->v_logicalDevice->get());

        constexpr VkDeviceSize vbSize = MAX_VERTICES * sizeof(Vertex2D);
        this->v_vertexBuffer[i] = std::make_unique<VulkanBuffer>(
            this->v_logicalDevice->get(), this->v_physicalDevice->get(), vbSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    if (!this->v_indexBuffer) {
        constexpr uint32_t indexCount = MAX_INDICES;
        std::vector<uint16_t> indices(indexCount);
        for (uint32_t q = 0; q < MAX_COMMANDS; ++q) {
            const uint16_t base = static_cast<uint16_t>(q * 4);
            indices[q * 6 + 0] = base + 0;
            indices[q * 6 + 1] = base + 1;
            indices[q * 6 + 2] = base + 2;
            indices[q * 6 + 3] = base + 0;
            indices[q * 6 + 4] = base + 2;
            indices[q * 6 + 5] = base + 3;
        }
        this->v_indexBuffer = std::make_unique<VulkanBuffer>(
            this->v_logicalDevice->get(), this->v_physicalDevice->get(),
            indexCount * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        this->v_indexBuffer->upload(indices.data(), indices.size() * sizeof(uint16_t));
    }

    this->m_pipelineManager->registerAll(this->v_renderPass->get());
}
