#ifndef F38C37D3_947C_4BF0_9EE4_4751E93A423D
#define F38C37D3_947C_4BF0_9EE4_4751E93A423D

#include "glm/ext/vector_float2.hpp"
#include <vector>

struct Particle
{
    glm::vec2 position;
    glm::vec2 velocity;
    float lifetime{1.0f};
    float maxLifetime{1.0f};
    float size{1.0f};
    float rotation{0.0f};
    float rotationSpeed{0.0f};
};

class ParticleInterface
{
  public:
    ParticleInterface() = default;
    ~ParticleInterface() = default;

    virtual void onBegin() = 0;
    virtual void onUpdate() = 0;
    virtual void onDraw() = 0;
    virtual void onEnd() = 0;

  private:
    std::vector<Particle> m_particle;
};

#endif /* F38C37D3_947C_4BF0_9EE4_4751E93A423D */
