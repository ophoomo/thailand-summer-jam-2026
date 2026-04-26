
#include "game/cards/hand.h"
#include "audio/audio_interface.h"
#include "game/cards/card.h"
#include "utils/logger.h"
#include <memory>

// ============================================================
// Construction / destruction
// ============================================================

CardHand::CardHand(std::shared_ptr<OxRenderer> m_renderer,
                   std::shared_ptr<AssetsInterface> m_assets,
                   std::shared_ptr<AudioInterface> m_audio)
{
    LOG_TRACE("[CardHand] Initializing");
    for (int i = 0; i < MAX_CARD_SLOTS; i++) {
        this->m_cards[i] = std::make_unique<Card>(m_renderer);
    }
    this->m_renderer = m_renderer;
    this->m_assets = m_assets;
    this->m_audio = m_audio;
}

CardHand::~CardHand()
{
    LOG_TRACE("[CardHand] Destroy");
    this->m_renderer->freeTexture("card_defend");
    this->m_renderer->freeTexture("card_attack");
    this->m_renderer->freeTexture("card_skill");
    this->m_renderer->freeTexture("shadow_veil");
    this->m_renderer->freeTexture("arcane_explosion");
    this->m_renderer->freeTexture("cursed_moonstrike");
    this->m_renderer->freeTexture("full_moon");
    this->m_renderer->freeTexture("lunar_beam");
    this->m_renderer->freeTexture("lunar_empowerment");
    this->m_renderer->freeTexture("lunar_heal");
    this->m_renderer->freeTexture("lunar_shield");
    this->m_audio->unload("card_slide_sfx_1");
    this->m_audio->unload("card_remove_sfx_1");
}

// ============================================================
// Public Methods
// ============================================================

void CardHand::onEnter()
{
    // Load Image
    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/card/card_defend.png", w, h, c);
    this->m_renderer->createTexture("card_defend", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/card_attack.png", w, h, c);
    this->m_renderer->createTexture("card_attack", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/card_skill.png", w, h, c);
    this->m_renderer->createTexture("card_skill", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/shadow_veil.png", w, h, c);
    this->m_renderer->createTexture("shadow_veil", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/arcane_explosion.png", w, h, c);
    this->m_renderer->createTexture("arcane_explosion", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/cursed_moonstrike.png", w, h, c);
    this->m_renderer->createTexture("cursed_moonstrike", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/full_moon.png", w, h, c);
    this->m_renderer->createTexture("full_moon", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/lunar_beam.png", w, h, c);
    this->m_renderer->createTexture("lunar_beam", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/lunar_empowerment.png", w, h, c);
    this->m_renderer->createTexture("lunar_empowerment", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/lunar_heal.png", w, h, c);
    this->m_renderer->createTexture("lunar_heal", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/card/lunar_shield.png", w, h, c);
    this->m_renderer->createTexture("lunar_shield", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    // Load Image
    int channels, sample_rate;
    short *data;
    auto samples = this->m_assets->loadAudio("assets/audio/card/card_slide_sfx.ogg", channels,
                                             sample_rate, data);
    this->m_audio->load("card_slide_sfx_1", channels, samples, sample_rate, data);

    samples = this->m_assets->loadAudio("assets/audio/card/card_remove_sfx.ogg", channels,
                                        sample_rate, data);
    this->m_audio->load("card_remove_sfx_1", channels, samples, sample_rate, data);

    m_db.load("assets/scripts/game/cards/cards.lua", this->m_assets);

    // Start all cards hidden; syncWithBattle will populate them from battle state
    for (int i = 0; i < static_cast<int>(MAX_CARD_SLOTS); i++) {
        CardInfo empty;
        empty.id = i;
        empty.show = false;
        this->m_cards[i]->onEnter(empty, WIDTH_CARD, HEIGHT_CARD);
        m_synced_ids[i] = -1;
    }
    calculatePositionCardInHand();
}

void CardHand::onDraw()
{
    for (auto &card : this->m_cards) {
        card->onDraw();
    }
}

void CardHand::onUpdate(double dt, float mouse_x, float mouse_y, bool mouse_clicked,
                        bool right_clicked)
{
    this->m_mouse_x = mouse_x;
    this->m_mouse_y = mouse_y;
    this->m_mouse_clicked = mouse_clicked;
    this->m_right_clicked = right_clicked;

    if (m_sfx_cooldown > 0.0f)
        m_sfx_cooldown -= static_cast<float>(dt);

    this->calculatePositionCardInHand();
    for (auto &card : this->m_cards) {
        card->onUpdate(dt);
    }
}

// ============================================================
// Battle Sync
// ============================================================

void CardHand::syncWithBattle(const battle::HandComp &hand)
{
    for (int i = 0; i < static_cast<int>(MAX_CARD_SLOTS); i++) {
        int32_t new_id = (i < hand.count) ? hand.slots[static_cast<size_t>(i)] : -1;
        if (new_id == m_synced_ids[i])
            continue;
        m_synced_ids[i] = new_id;
        if (new_id >= 0) {
            const CardInfo *info = m_db.findByIndex(new_id);
            if (info) {
                m_cards[i]->setSelected(false);
                m_cards[i]->showCard(*info);
            }
        } else {
            m_cards[i]->setSelected(false);
            m_cards[i]->hideCard();
        }
    }
}

int CardHand::getSelectedSlot() const
{
    for (int i = 0; i < static_cast<int>(MAX_CARD_SLOTS); i++) {
        if (m_cards[i]->getSelected())
            return i;
    }
    return -1;
}

void CardHand::clearSelection()
{
    for (auto &card : m_cards)
        card->setSelected(false);
}

std::pair<float, float> CardHand::getSelectedCardCenter() const
{
    for (int i = 0; i < static_cast<int>(MAX_CARD_SLOTS); i++) {
        if (m_cards[i]->getSelected())
            return {m_cards[i]->getCenterX(), m_cards[i]->getCenterY()};
    }
    return {-1.0f, -1.0f};
}

int CardHand::consumeDiscardedSlot()
{
    int s = m_last_discarded_slot;
    m_last_discarded_slot = -1;
    return s;
}

// ============================================================
// Private Methods
// ============================================================

void CardHand::calculatePositionCardInHand()
{
    int n = 0;
    for (int i = 0; i < MAX_CARD_SLOTS; i++) {
        if (this->m_cards[i]->isInHand())
            n++;
    }

    if (n == 0)
        return;

    float totalWidth = (n * WIDTH_CARD) + ((n - 1) * CARD_GAP);
    float startX = CENTER_HAND - (totalWidth / 2.0f);

    int slot = 0;
    for (int i = 0; i < MAX_CARD_SLOTS; i++) {
        if (!this->m_cards[i]->isInHand()) {
            this->m_cards[i]->setOnHover(false);
            continue;
        }

        float cardX = startX + slot * (WIDTH_CARD + CARD_GAP);
        slot++;

        // Use card center for accurate 3D sound placement
        float cardCenterX = cardX + WIDTH_CARD * 0.5f;
        float soundX = std::max(
            -3.0f, std::min(3.0f, ((cardCenterX - CENTER_HAND) / (totalWidth * 0.5f)) * 2.0f));

        this->m_cards[i]->setPosition(cardX, 450);

        bool hovered = this->m_cards[i]->checkHover(this->m_mouse_x, this->m_mouse_y);

        // Expand the click area by the glow border size so clicking the yellow
        // box rim deselects the card even when the cursor is outside the sprite.
        float click_pad = this->m_cards[i]->getSelected() ? 14.0f : 0.0f;
        bool click_hit =
            m_mouse_clicked && this->m_cards[i]->checkHover(m_mouse_x, m_mouse_y, click_pad);

        if (hovered) {
            if (!this->m_cards[i]->getOnHover() && m_sfx_cooldown <= 0.0f) {
                this->m_audio->play_sfx_3d("card_slide_sfx_1", soundX, 0.0f, -3.0f, 0.4);
                m_sfx_cooldown = 0.1f;
            }
            this->m_cards[i]->setOnHover(true);
        } else {
            this->m_cards[i]->setOnHover(false);
        }

        if (click_hit) {
            bool was_selected = this->m_cards[i]->getSelected();
            for (int j = 0; j < MAX_CARD_SLOTS; j++) {
                this->m_cards[j]->setSelected(false);
            }
            if (!was_selected) {
                this->m_cards[i]->setSelected(true);
                // Swap for a dedicated select SFX when available
                this->m_audio->play_sfx_3d("card_slide_sfx_1", soundX, 0.0f, -3.0f, 0.65);
            }
        }

        bool right_hit = m_right_clicked && this->m_cards[i]->checkHover(m_mouse_x, m_mouse_y);
        if (right_hit) {
            m_last_discarded_slot = i;
            this->m_cards[i]->discard();
            this->m_audio->play_sfx_3d("card_remove_sfx_1", soundX, 0.0f, -3.0f, 0.65);
        }
    }
}
