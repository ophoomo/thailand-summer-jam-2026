
#include "renderer/vulkan/vulkan_pipeline_manager.h"

// ============================================================
// Construction / destruction
// ============================================================

VulkanPipelineManager::VulkanPipelineManager(std::shared_ptr<VulkanLogicalDevice> device,
                                             std::shared_ptr<VulkanShaderManager> shaderManager)
{
    this->m_device = device;
    this->m_shaderManager = shaderManager;
    this->createSharedLayout();
}

VulkanPipelineManager::~VulkanPipelineManager()
{
    this->destroyPipelines();

    VkDevice dev = m_device->get();
    if (this->m_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(dev, this->m_layout, nullptr);
    if (this->m_descriptorLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(dev, this->m_descriptorLayout, nullptr);
    if (this->m_textLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(dev, this->m_textLayout, nullptr);
    if (this->m_textDescriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(dev, this->m_textDescriptorSetLayout, nullptr);
}

// ============================================================
// Public Methods
// ============================================================

void VulkanPipelineManager::registerAll(VkRenderPass renderPass)
{
    this->destroyPipelines();

    const ShaderID defaultVert = this->m_shaderManager->load("assets/shaders/batch2d.vert.spv");
    const ShaderID defaultFrag = this->m_shaderManager->load("assets/shaders/batch2d.frag.spv");

    const ShaderID msdfFrag = this->m_shaderManager->load("assets/shaders/msdf_text.frag.spv");
    const ShaderID spriteFrag = this->m_shaderManager->load("assets/shaders/sprite.frag.spv");
    const ShaderID circleFrag = this->m_shaderManager->load("assets/shaders/circle.frag.spv");

    // Non-text pipelines use the base layout (no descriptor sets).
    const std::pair<PipelineID, PipelineDesc> descs[] = {
        {PipelineID::Quad, {defaultVert, defaultFrag, BlendMode::None}},
        {PipelineID::AlphaQuad, {defaultVert, defaultFrag, BlendMode::Alpha}},
        {PipelineID::Text, {defaultVert, defaultFrag, BlendMode::Alpha}},
        {PipelineID::Particle, {defaultVert, defaultFrag, BlendMode::Additive}},
        {PipelineID::Circle, {defaultVert, circleFrag, BlendMode::Alpha}},
    };
    for (const auto &[id, desc] : descs) {
        this->m_pipelines[static_cast<uint8_t>(id)] = this->buildPipeline(desc, renderPass);
    }

    // Text pipeline — MSDF fragment shader, texture-sampler layout.
    this->m_pipelines[static_cast<uint8_t>(PipelineID::Text)] = this->buildPipeline(
        {defaultVert, msdfFrag, BlendMode::Alpha}, renderPass, this->m_textLayout);

    // Sprite pipeline — RGBA texture sampling, same layout as Text.
    this->m_pipelines[static_cast<uint8_t>(PipelineID::Sprite)] = this->buildPipeline(
        {defaultVert, spriteFrag, BlendMode::Alpha}, renderPass, this->m_textLayout);

    this->m_shaderManager->unload(defaultVert);
    this->m_shaderManager->unload(defaultFrag);
    this->m_shaderManager->unload(msdfFrag);
    this->m_shaderManager->unload(spriteFrag);
    this->m_shaderManager->unload(circleFrag);
}

VkPipeline VulkanPipelineManager::getPipeline(PipelineID id) const
{
    auto it = m_pipelines.find(static_cast<uint8_t>(id));
    return (it != m_pipelines.end()) ? it->second : VK_NULL_HANDLE;
}

// ============================================================
// Private Methods
// ============================================================

void VulkanPipelineManager::createSharedLayout()
{
    VkDevice dev = m_device->get();

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(float) * 2; // float2 screenSize

    // ── Base layout (Quad / AlphaQuad / Particle) — no descriptor sets ──────
    {
        VkPipelineLayoutCreateInfo plInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plInfo.setLayoutCount = 0;
        plInfo.pushConstantRangeCount = 1;
        plInfo.pPushConstantRanges = &pushRange;
        if (vkCreatePipelineLayout(dev, &plInfo, nullptr, &m_layout) != VK_SUCCESS)
            throw std::runtime_error(
                "VulkanPipelineManager: failed to create base pipeline layout");
    }

    // Push constant ranges for the text pipeline:
    //   [0] vertex   offset= 0 size= 8 → float2 screenSize
    //   [1] fragment offset= 8 size=36 → float2 unitRange
    //                                    float4 effectColor  (offset 16)
    //                                    float  effectParam0 (offset 32)
    //                                    float  effectParam1 (offset 36)
    //                                    int    effectType   (offset 40)
    const VkPushConstantRange textPushRanges[2] = {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2},
        {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float) * 2, sizeof(float) * 2 + sizeof(TextEffectGPU)},
    };

    // ── Text descriptor set layout — set 0, binding 0: combined image sampler
    {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo dslInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        dslInfo.bindingCount = 1;
        dslInfo.pBindings = &samplerBinding;
        if (vkCreateDescriptorSetLayout(dev, &dslInfo, nullptr, &m_textDescriptorSetLayout) !=
            VK_SUCCESS)
            throw std::runtime_error(
                "VulkanPipelineManager: failed to create text descriptor set layout");
    }

    // ── Text pipeline layout — push constant + texture descriptor ────────────
    {
        VkPipelineLayoutCreateInfo plInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plInfo.setLayoutCount = 1;
        plInfo.pSetLayouts = &m_textDescriptorSetLayout;
        plInfo.pushConstantRangeCount = 2;
        plInfo.pPushConstantRanges = textPushRanges;
        if (vkCreatePipelineLayout(dev, &plInfo, nullptr, &m_textLayout) != VK_SUCCESS)
            throw std::runtime_error(
                "VulkanPipelineManager: failed to create text pipeline layout");
    }
}

VkPipeline VulkanPipelineManager::buildPipeline(const PipelineDesc &desc, VkRenderPass rp,
                                                VkPipelineLayout layout) const
{
    // ── Shader stages ────────────────────────────────────────────────────────
    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = this->m_shaderManager->module(desc.vertShader);
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = this->m_shaderManager->module(desc.fragShader);
    stages[1].pName = "main";

    // ── Vertex input — mirrors Vertex2D memory layout ────────────────────────
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(Vertex2D);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[4] = {};
    attrs[0] = {0, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex2D, x))};
    attrs[1] = {1, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex2D, u))};
    attrs[2] = {2, 0, VK_FORMAT_R8G8B8A8_UNORM, static_cast<uint32_t>(offsetof(Vertex2D, color))};
    attrs[3] = {3, 0, VK_FORMAT_R32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex2D, tex_slot))};

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &binding;
    vi.vertexAttributeDescriptionCount = 4;
    vi.pVertexAttributeDescriptions = attrs;

    // ── Input assembly ───────────────────────────────────────────────────────
    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // ── Viewport / scissor (dynamic — set per-frame after resize) ───────────
    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    const VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn{};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    // ── Rasterization ────────────────────────────────────────────────────────
    VkPipelineRasterizationStateCreateInfo ras{};
    ras.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ras.polygonMode = VK_POLYGON_MODE_FILL;
    ras.cullMode = VK_CULL_MODE_NONE; // sprites may be horizontally flipped
    ras.frontFace = VK_FRONT_FACE_CLOCKWISE;
    ras.lineWidth = 1.0f;

    // ── Multisample (1x — no MSAA for 2D pixel art) ─────────────────────────
    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // ── Depth / stencil (disabled — 2D uses sort-key painter's algorithm) ───
    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // ── Color blending ───────────────────────────────────────────────────────
    const VkPipelineColorBlendAttachmentState blendAttach = makeBlendAttachment(desc.blend);
    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &blendAttach;

    // ── Assemble ─────────────────────────────────────────────────────────────
    VkGraphicsPipelineCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &vi;
    info.pInputAssemblyState = &ia;
    info.pViewportState = &vp;
    info.pRasterizationState = &ras;
    info.pMultisampleState = &ms;
    info.pDepthStencilState = &ds;
    info.pColorBlendState = &cb;
    info.pDynamicState = &dyn;
    info.layout = (layout != VK_NULL_HANDLE) ? layout : this->m_layout;
    info.renderPass = rp;
    info.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(m_device->get(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) !=
        VK_SUCCESS)
        throw std::runtime_error("VulkanPipelineManager: vkCreateGraphicsPipelines failed");
    return pipeline;
}

VkPipelineColorBlendAttachmentState VulkanPipelineManager::makeBlendAttachment(BlendMode mode)
{
    VkPipelineColorBlendAttachmentState s{};
    s.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    switch (mode) {
    case BlendMode::None:
        s.blendEnable = VK_FALSE;
        break;

    case BlendMode::Alpha:
        // Standard pre-multiplied-alpha composite: out = src*srcA + dst*(1-srcA)
        s.blendEnable = VK_TRUE;
        s.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        s.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        s.colorBlendOp = VK_BLEND_OP_ADD;
        s.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        s.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        s.alphaBlendOp = VK_BLEND_OP_ADD;
        break;

    case BlendMode::Additive:
        // out = src*srcA + dst — bright effects accumulate without darkening.
        s.blendEnable = VK_TRUE;
        s.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        s.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        s.colorBlendOp = VK_BLEND_OP_ADD;
        s.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        s.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        s.alphaBlendOp = VK_BLEND_OP_ADD;
        break;

    case BlendMode::Multiply:
        // out = src*dst — darkens based on source colour (shadows, colour grading).
        s.blendEnable = VK_TRUE;
        s.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
        s.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        s.colorBlendOp = VK_BLEND_OP_ADD;
        s.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        s.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        s.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    }
    return s;
}

void VulkanPipelineManager::destroyPipelines()
{
    VkDevice dev = m_device->get();
    for (auto &[id, pipeline] : m_pipelines) {
        if (pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(dev, pipeline, nullptr);
    }
    this->m_pipelines.clear();
}
