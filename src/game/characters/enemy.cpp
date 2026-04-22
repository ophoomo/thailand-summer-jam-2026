
#include "game/characters/enemy.h"
#include "game/battle/components.h"
#include "utils/logger.h"
#include <format>
#include <memory>
#include <utility>

// ============================================================
// Construction / destruction
// ============================================================

Enemy::Enemy(std::shared_ptr<OxRenderer> renderer, std::shared_ptr<AssetsInterface> assets)
    : m_renderer(std::move(renderer)), m_assets(std::move(assets))
{
    LOG_TRACE("[Enemy] Initializing");
}

Enemy::~Enemy()
{
    LOG_TRACE("[Enemy] Destroy");
    m_renderer->freeTexture("enemy1");
    m_renderer->freeTexture("enemy2");
    m_renderer->freeTexture("boss");
}

// ============================================================
// Public Methods
// ============================================================

void Enemy::onEnter()
{
    LOG_TRACE("[Player] onEnter");

    // Load enemy sprites for sprite-based rendering
    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/enemy1.png", w, h, c);
    this->m_renderer->createTexture("enemy1", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/enemy2.png", w, h, c);
    this->m_renderer->createTexture("enemy2", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/ิboss.png", w, h, c);
    this->m_renderer->createTexture("boss", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    // Animation
    Animation idle("idle", true);
    idle.buildFromSheet(6, 2, 0, 0, 4, 0.1f);

    Animation attak("attack", true);
    attak.buildFromSheet(6, 2, 1, 0, 4, 0.1f);

    this->m_animator->addAnimation(std::move(idle));
    this->m_animator->addAnimation(std::move(attak));
    this->m_animator->play("idle");
}

void Enemy::onDraw(entt::registry &reg, entt::entity enemy_entity)
{
    auto *sprite = reg.try_get<battle::SpriteComp>(enemy_entity);
    auto *hp = reg.try_get<battle::HealthComp>(enemy_entity);

    if (!sprite || sprite->texture_id.empty()) {
        return; // No sprite data available
    }

    if (const AnimFrame *f = m_animator->currentFrame()) {
        m_renderer->oxDrawSpriteSheet(sprite->x, sprite->y, WIDTH_ENEMY, HEIGHT_ENEMY,
                                      sprite->texture_id, f->u0, f->v0, f->u1, f->v1,
                                      {255, 255, 255, 255});
    }
}
