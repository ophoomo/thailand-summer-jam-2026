#ifndef B9D71DE3_333E_4754_AE6C_A73D4499BE7F
#define B9D71DE3_333E_4754_AE6C_A73D4499BE7F

#include "renderer/text_effect.h"
#include "vertex.h"

enum RendererAPI : uint8_t { Vulkan };

enum class PipelineID : uint8_t {
    Quad = 0,
    AlphaQuad = 1,
    Text = 2,
    Particle = 3,
    Sprite = 4,
    Circle = 5,
    Count
};

// Opaque handle to a GPU texture.  0 is reserved as the null / invalid value.
using TextureHandle = uint32_t;
static constexpr TextureHandle INVALID_TEXTURE = 0;

class RendererInterface
{
  public:
    RendererInterface() = default;
    virtual ~RendererInterface() = default;

    virtual void resize(int width, int height) = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SubmitVertices(const Vertex2D *verts, uint32_t quadCount, PipelineID pipeline) = 0;
    virtual void SubmitTextVertices(const Vertex2D *verts, uint32_t quadCount,
                                    TextureHandle texture, const TextEffectGPU &effect) = 0;
    virtual void SubmitSpriteVertices(const Vertex2D *verts, uint32_t quadCount,
                                      TextureHandle texture) = 0;

    virtual TextureHandle createTexture(const uint8_t *pixels, int width, int height) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual void waitIdle() {}

    virtual void setMsdfUnitRange(float x, float y) {}

    virtual void initImGui(void * /*sdlWindow*/) {}
    virtual void shutdownImGui() {}
};

#endif /* B9D71DE3_333E_4754_AE6C_A73D4499BE7F */
