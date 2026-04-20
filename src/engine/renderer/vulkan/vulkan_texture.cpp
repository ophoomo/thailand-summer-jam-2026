
#include "renderer/vulkan/vulkan_texture.h"
#include "utils/logger.h"

// ── Local RAII guards ─────────────────────────────────────────────────────────
// Used only inside the constructor to ensure temporary Vulkan resources are
// always freed, even when an exception propagates out.

namespace {

struct StagingGuard
{
    VkDevice dev;
    VkBuffer buf = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;

    ~StagingGuard()
    {
        if (buf != VK_NULL_HANDLE)
            vkDestroyBuffer(dev, buf, nullptr);
        if (mem != VK_NULL_HANDLE)
            vkFreeMemory(dev, mem, nullptr);
    }
};

struct CmdPoolGuard
{
    VkDevice dev;
    VkCommandPool pool = VK_NULL_HANDLE;

    ~CmdPoolGuard()
    {
        if (pool != VK_NULL_HANDLE)
            vkDestroyCommandPool(dev, pool, nullptr);
    }
};

} // namespace

// ============================================================
// Construction / destruction
// ============================================================

VulkanTexture::VulkanTexture(const UploadContext &ctx, const uint8_t *pixels, int width, int height)
    : m_device(ctx.device)
{
    const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;

    // ── Staging buffer (host-visible) ────────────────────────────────────────
    // Guard ensures cleanup on any exit path, including exceptions.
    StagingGuard staging{m_device};
    {
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = imageSize;
        bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(m_device, &bi, nullptr, &staging.buf) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to create staging buffer");

        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(m_device, staging.buf, &req);

        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(ctx.physDevice, req.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_device, &ai, nullptr, &staging.mem) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to allocate staging memory");

        vkBindBufferMemory(m_device, staging.buf, staging.mem, 0);

        void *mapped = nullptr;
        if (vkMapMemory(m_device, staging.mem, 0, imageSize, 0, &mapped) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to map staging memory");
        std::memcpy(mapped, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, staging.mem);
    }

    // ── VkImage (device-local, optimal tiling) ───────────────────────────────
    {
        VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ii.imageType = VK_IMAGE_TYPE_2D;
        ii.format = VK_FORMAT_R8G8B8A8_UNORM;
        ii.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        ii.mipLevels = 1;
        ii.arrayLayers = 1;
        ii.samples = VK_SAMPLE_COUNT_1_BIT;
        ii.tiling = VK_IMAGE_TILING_OPTIMAL;
        ii.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(m_device, &ii, nullptr, &m_image) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to create image");

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(m_device, m_image, &req);

        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = req.size;
        ai.memoryTypeIndex =
            findMemoryType(ctx.physDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(m_device, &ai, nullptr, &m_memory) != VK_SUCCESS) {
            // m_image allocated, destructor won't run — clean up manually.
            vkDestroyImage(m_device, m_image, nullptr);
            m_image = VK_NULL_HANDLE;
            throw std::runtime_error("[VulkanTexture] Failed to allocate image memory");
        }

        vkBindImageMemory(m_device, m_image, m_memory, 0);
    }

    // ── One-time transfer: stage → device image ──────────────────────────────
    {
        CmdPoolGuard cmdGuard{m_device};

        VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        ci.queueFamilyIndex = ctx.queueFamily;
        if (vkCreateCommandPool(m_device, &ci, nullptr, &cmdGuard.pool) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to create transfer command pool");

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        ai.commandPool = cmdGuard.pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(m_device, &ai, &cmd) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to allocate transfer command buffer");

        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &bi);

        transitionImageLayout(cmd, m_image, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        vkCmdCopyBufferToImage(cmd, staging.buf, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        transitionImageLayout(cmd, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        vkQueueSubmit(ctx.queue, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(ctx.queue);
        // cmdGuard destroyed here → vkDestroyCommandPool
    }
    // staging destroyed here → vkDestroyBuffer + vkFreeMemory

    // ── VkImageView + VkSampler ──────────────────────────────────────────────
    // If either fails, catch and release already-allocated image resources
    // (destructor won't run because we're still inside the constructor).
    try {
        VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vi.image = m_image;
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = VK_FORMAT_R8G8B8A8_UNORM;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.layerCount = 1;
        vi.subresourceRange.levelCount = 1;
        if (vkCreateImageView(m_device, &vi, nullptr, &m_imageView) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to create image view");

        VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        si.magFilter = VK_FILTER_LINEAR;
        si.minFilter = VK_FILTER_LINEAR;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        si.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.minLod = 0.0f;
        si.maxLod = 0.0f;
        if (vkCreateSampler(m_device, &si, nullptr, &m_sampler) != VK_SUCCESS)
            throw std::runtime_error("[VulkanTexture] Failed to create sampler");
    } catch (...) {
        if (m_imageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_device, m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE)
            vkDestroyImage(m_device, m_image, nullptr);
        if (m_memory != VK_NULL_HANDLE)
            vkFreeMemory(m_device, m_memory, nullptr);
        throw;
    }

    LOG_CORE_TRACE("[VulkanTexture] Uploaded {}×{} px to GPU", width, height);
}

VulkanTexture::~VulkanTexture()
{
    if (m_sampler != VK_NULL_HANDLE)
        vkDestroySampler(m_device, m_sampler, nullptr);
    if (m_imageView != VK_NULL_HANDLE)
        vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_image != VK_NULL_HANDLE)
        vkDestroyImage(m_device, m_image, nullptr);
    if (m_memory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_memory, nullptr);
}

// ============================================================
// Private Methods
// ============================================================

uint32_t VulkanTexture::findMemoryType(VkPhysicalDevice physDevice, uint32_t filter,
                                       VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &props);
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        if ((filter & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    throw std::runtime_error("[VulkanTexture] No suitable memory type found");
}

void VulkanTexture::transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout from,
                                          VkImageLayout to)
{
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = from;
    barrier.newLayout = to;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkPipelineStageFlags srcStage, dstStage;
    if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
