#ifndef B6C12FB5_F909_493B_8B63_31C5DB2C25F5
#define B6C12FB5_F909_493B_8B63_31C5DB2C25F5

#include "animation/animator.h"
#include "assets/assets_interface.h"
#include "renderer/ox_renderer.h"
#include <memory>
#include <string>

// Enemy sprite dimensions — match player dimensions
static constexpr float WIDTH_ENEMY = 104.54f;
static constexpr float HEIGHT_ENEMY = 151.0f;

class Enemy
{
  public:
    Enemy(std::shared_ptr<OxRenderer> renderer, std::shared_ptr<AssetsInterface> assets);
    ~Enemy();

    void onEnter();
    void onDraw(entt::registry &reg, entt::entity enemy_entity);

    float getWidth() const
    {
        return WIDTH_ENEMY;
    }
    float getHeight() const
    {
        return HEIGHT_ENEMY;
    }

  private:
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::unique_ptr<Animator> m_animator;
};

#endif /* B6C12FB5_F909_493B_8B63_31C5DB2C25F5 */
