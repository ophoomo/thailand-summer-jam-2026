#ifndef B863F30F_2149_44DC_B37E_CC3C7EB9AA24
#define B863F30F_2149_44DC_B37E_CC3C7EB9AA24

#include "animation/animator.h"
#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "game/battle/battle_system.h"
#include "renderer/ox_renderer.h"
#include <memory>

static constexpr float WIDTH_PLAYER = 156.81f;
static constexpr float HEIGHT_PLAYER = 226.5f;

class Player
{
  public:
    Player(std::shared_ptr<OxRenderer> renderer, std::shared_ptr<AssetsInterface> assets,
           std::shared_ptr<AudioInterface> audio, std::shared_ptr<battle::BattleSystem> m_battle);
    ~Player();

    void onEnter();
    void onDraw();
    void onUpdate(double deltaTime);

    float getX() const
    {
        return this->x;
    }
    float getY() const
    {
        return this->y;
    }
    float getWidth() const
    {
        return WIDTH_PLAYER;
    }
    float getHeight() const
    {
        return HEIGHT_PLAYER;
    }

  private:
    void onAttack(int posIndex);
    void onDamaged(int posIndex);

    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;
    std::unique_ptr<Animator> m_animator;
    std::shared_ptr<battle::BattleSystem> m_battle;

    float x = 180, y = 250;

    float m_health = 10.0;
};

#endif /* B863F30F_2149_44DC_B37E_CC3C7EB9AA24 */
