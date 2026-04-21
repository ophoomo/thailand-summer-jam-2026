#ifndef A9DE25F5_1BDA_4842_BE66_51F353747AEF
#define A9DE25F5_1BDA_4842_BE66_51F353747AEF

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "game/battle/components.h"
#include "game/cards/card.h"
#include "game/cards/card_database.h"
#include <memory>

static constexpr float WIDTH_CARD = 180.0f;
static constexpr float HEIGHT_CARD = 266.0f;

static constexpr uint32_t MAX_CARD_SLOTS = 4;
static constexpr float CENTER_HAND = 782;
static constexpr float CARD_GAP = 14;

class CardHand
{
  public:
    CardHand(std::shared_ptr<OxRenderer> m_renderer, std::shared_ptr<AssetsInterface> m_assets,
             std::shared_ptr<AudioInterface> m_audio);
    ~CardHand();

    void onEnter();
    void onDraw();
    void onUpdate(double dt, float mouse_x, float mouse_y, bool mouse_clicked, bool right_clicked);

    // Sync visual cards with the battle hand state (call each frame after battle update)
    void syncWithBattle(const battle::HandComp& hand);

    // Returns the currently selected card slot index, or -1 if none
    int getSelectedSlot() const;
    void clearSelection();

    // Returns the visual center of the selected card, or {-1,-1} if none selected
    std::pair<float, float> getSelectedCardCenter() const;

    // Returns the slot that was right-click discarded this frame, or -1 if none.
    // Consuming resets the value.
    int consumeDiscardedSlot();

  private:
    void calculatePositionCardInHand();

    std::unique_ptr<Card> m_cards[MAX_CARD_SLOTS];
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;

    float m_mouse_x = 0;
    float m_mouse_y = 0;
    bool m_mouse_clicked = false;
    bool m_right_clicked = false;
    float m_sfx_cooldown = 0.0f;

    CardDatabase m_db;

    // Tracks the battle card_id currently displayed in each visual slot (-1 = hidden)
    int32_t m_synced_ids[MAX_CARD_SLOTS] = {-1, -1, -1, -1};
    int m_last_discarded_slot = -1;
};

#endif /* A9DE25F5_1BDA_4842_BE66_51F353747AEF */
