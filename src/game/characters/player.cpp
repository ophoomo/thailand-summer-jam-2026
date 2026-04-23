
#include "game/characters/player.h"
#include "game/battle/battle_system.h"
#include "renderer/color.h"
#include "utils/logger.h"
#include <cstdlib>
#include <format>
#include <memory>
#include <utility>

// ============================================================
// Construction / destruction
// ============================================================

Player::Player(std::shared_ptr<OxRenderer> renderer, std::shared_ptr<AssetsInterface> assets,
               std::shared_ptr<AudioInterface> audio,
               std::shared_ptr<battle::BattleSystem> m_battle)
{
    LOG_TRACE("[Player] Initializing");
    this->m_renderer = renderer;
    this->m_assets = assets;
    this->m_audio = audio;
    this->m_animator = std::make_unique<Animator>();
    this->m_battle = m_battle;
}

Player::~Player()
{
    LOG_TRACE("[Player] Destroy");
    this->m_renderer->freeTexture("player");
    for (int i = 1; i <= 3; i++) {

        std::string name = std::format("player_damaged_{}", i);
        this->m_audio->unload(name);
    }
}

// ============================================================
// Public Methods
// ============================================================

void Player::onEnter()
{
    LOG_TRACE("[Player] onEnter");

    // Load Image
    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/player.png", w, h, c);
    this->m_renderer->createTexture("player", pixel, w, h);

    // Load Audio
    int channels, sample_rate, sample;
    short *data;

    for (int i = 1; i <= 3; i++) {
        std::string path = std::format("assets/audio/player/damaged{}_sfx.ogg", i);
        sample = this->m_assets->loadAudio(path.c_str(), channels, sample_rate, data);

        std::string name = std::format("player_damaged_{}", i);
        this->m_audio->load(name, channels, sample, sample_rate, data);
    }

    // Animation
    Animation idle("idle", true);
    idle.buildFromSheet(6, 2, 0, 0, 4, 0.1f);

    Animation attak("attack", true);
    attak.buildFromSheet(6, 2, 1, 0, 4, 0.1f);

    this->m_animator->addAnimation(std::move(idle));
    this->m_animator->addAnimation(std::move(attak));
    this->m_animator->play("idle");
}

void Player::onUpdate(double deltaTime)
{
    this->m_animator->onUpdate(static_cast<float>(deltaTime));
}

void Player::onDraw()
{
    auto &reg = this->m_battle->getRegistry();
    entt::entity player = this->m_battle->getPlayer();
    auto *hp = reg.try_get<battle::HealthComp>(player);
    uint8_t hp_opacity = hp->ratio() * 255;

    // ── Name ─────────────────────────────────────────────────────────────
    std::string name = "Selenia";
    float tw = this->m_renderer->measureText(name.c_str(), 12);
    this->m_renderer->oxDrawText(this->x + (this->getWidth() / 2) - (tw / 2), this->y - 20,
                                 name.c_str(), 12, Color::White(),
                                 TextEffect::Outline(Color::Black()), 2);

    // Block
    if (auto *bl = reg.try_get<battle::BlockComp>(player); bl && bl->amount > 0) {
        std::string t = std::format("BLK  {}", bl->amount);
        this->m_renderer->oxDrawText(this->x, this->y - 40, t.c_str(), 11, {100, 180, 255, 255},
                                     TextEffect::Outline(Color::Black()), 2);
    }

    if (const AnimFrame *f = m_animator->currentFrame()) {
        m_renderer->oxDrawSpriteSheet(this->x, this->y, WIDTH_PLAYER, HEIGHT_PLAYER, "player",
                                      f->u0, f->v0, f->u1, f->v1, {255, 255, 255, hp_opacity});
    }
}

// ============================================================
// Private Methods
// ============================================================

void Player::onDamaged(int posIndex)
{
    int soundIndex = std::rand() % 3 + 1;
    std::string soundName = "player_damaged_" + std::to_string(soundIndex);

    float x = 0.0f;
    if (posIndex == 0)
        x = -1.0f;
    else if (posIndex == 1)
        x = 0.0f;
    else
        x = 1.0f;

    this->m_audio->play_sfx_3d(soundName.c_str(), x, 0.0f, -2.0f);
}
