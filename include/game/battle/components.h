#ifndef BATTLE_COMPONENTS_H
#define BATTLE_COMPONENTS_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  Compile-time constants
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr int32_t HAND_CAPACITY = 4;
inline constexpr int32_t STARTER_HP = 80;
inline constexpr int32_t STARTER_ENERGY = 3;
inline constexpr int32_t MAX_LEVELS = 10;

// ─────────────────────────────────────────────────────────────────────────────
//  Core stat components  (8–12 bytes each — fits in one cache fetch)
// ─────────────────────────────────────────────────────────────────────────────

struct HealthComp
{
    int32_t current{0};
    int32_t max{0};

    [[nodiscard]] bool isDead() const noexcept
    {
        return current <= 0;
    }
    [[nodiscard]] float ratio() const noexcept
    {
        return max > 0 ? static_cast<float>(current) / static_cast<float>(max) : 0.0f;
    }
};

struct BlockComp
{
    int32_t amount{0}; // cleared at the start of each entity turn
};

struct EnergyComp
{
    int32_t current{0};
    int32_t max{STARTER_ENERGY};
};

struct StrengthComp
{
    int32_t value{0};
}; // +N to all attack damage
struct DexterityComp
{
    int32_t value{0};
}; // +N to all block gained

// ─────────────────────────────────────────────────────────────────────────────
//  Status effects  (6 bytes — single struct avoids multi-component lookup)
// ─────────────────────────────────────────────────────────────────────────────

struct StatusComp
{
    int8_t poison{0};     // lose N HP end-of-turn, then decrement
    int8_t burn{0};       // lose N HP end-of-turn (no decrement)
    int8_t vulnerable{0}; // +50% damage taken; decrement each turn
    int8_t weak{0};       // −25% damage dealt; decrement each turn
    int8_t frail{0};      // −25% block gained; decrement each turn
    int8_t regen{0};      // heal N HP end-of-turn
};

// ─────────────────────────────────────────────────────────────────────────────
//  Card management  (player only)
// ─────────────────────────────────────────────────────────────────────────────

struct DeckComp
{
    std::vector<int32_t> draw_pile; // card IDs (back = top)
    std::vector<int32_t> discard_pile;

    // Fisher-Yates shuffle of draw_pile
    void shuffle() noexcept
    {
        for (int32_t i = static_cast<int32_t>(draw_pile.size()) - 1; i > 0; --i) {
            int32_t j = rand() % (i + 1);
            std::swap(draw_pile[static_cast<size_t>(i)], draw_pile[static_cast<size_t>(j)]);
        }
    }

    // Move entire discard into draw_pile and reshuffle
    void refillFromDiscard() noexcept
    {
        draw_pile.insert(draw_pile.end(), discard_pile.begin(), discard_pile.end());
        discard_pile.clear();
        shuffle();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return draw_pile.empty() && discard_pile.empty();
    }
};

struct HandComp
{
    std::array<int32_t, HAND_CAPACITY> slots{};
    int32_t count{0};

    HandComp()
    {
        slots.fill(-1);
    }

    [[nodiscard]] bool isFull() const noexcept
    {
        return count >= HAND_CAPACITY;
    }
    [[nodiscard]] bool isEmpty() const noexcept
    {
        return count == 0;
    }

    void add(int32_t card_id) noexcept
    {
        if (isFull())
            return;
        slots[static_cast<size_t>(count++)] = card_id;
    }

    // Removes card at slot_index, shifts remaining cards left
    void removeAt(int32_t slot_index) noexcept
    {
        if (slot_index < 0 || slot_index >= count)
            return;
        for (int32_t i = slot_index; i < count - 1; ++i)
            slots[static_cast<size_t>(i)] = slots[static_cast<size_t>(i + 1)];
        slots[static_cast<size_t>(--count)] = -1;
    }

    // Discard entire hand into deck's discard pile
    void discardAll(DeckComp &deck) noexcept
    {
        for (int32_t i = 0; i < count; ++i)
            if (slots[static_cast<size_t>(i)] >= 0)
                deck.discard_pile.push_back(slots[static_cast<size_t>(i)]);
        slots.fill(-1);
        count = 0;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Enemy-specific components
// ─────────────────────────────────────────────────────────────────────────────

enum class EnemyType : uint8_t {
    SKELETON = 0,
    GOBLIN = 1,
    TROLL = 2,
    ARCHER = 3,
    DARK_KNIGHT = 4,
    NECROMANCER = 5,
    BOSS_DRAGON = 6,
    BOSS_LICH = 7,
    COUNT,
};

enum class IntentType : uint8_t {
    ATTACK,  // will deal damage
    DEFEND,  // will gain block
    BUFF,    // will strengthen self
    DEBUFF,  // will apply status to player
    SPECIAL, // boss ability
};

// What the enemy intends to do this turn (shown to player as UI hint)
struct IntentComp
{
    IntentType type{IntentType::ATTACK};
    int32_t damage{0}; // > 0 when attacking
    int32_t block{0};  // > 0 when defending
    int32_t times{1};  // multi-hit count
};

struct EnemyTypeComp
{
    EnemyType type{EnemyType::SKELETON};
    int32_t level{1};         // encounter level — scales stats
    int32_t pattern_index{0}; // cycles through AI pattern array
};

// ─────────────────────────────────────────────────────────────────────────────
//  Sprite rendering component
// ─────────────────────────────────────────────────────────────────────────────

struct SpriteComp
{
    std::string texture_id; // renderer texture identifier (e.g., "enemy1", "enemy2", "boss")
    float x{0};             // screen position
    float y{0};
    float width{0}; // sprite dimensions
    float height{0};
};

// ─────────────────────────────────────────────────────────────────────────────
//  Turn ordering
// ─────────────────────────────────────────────────────────────────────────────

struct TurnOrderComp
{
    int32_t priority{0}; // lower = acts earlier among enemies
};

// ─────────────────────────────────────────────────────────────────────────────
//  Level / progression  (attached to the player entity as a global context)
// ─────────────────────────────────────────────────────────────────────────────

struct LevelComp
{
    int32_t current{1};
    int32_t enemies_remaining{0};
    bool is_boss{false};
};

// ─────────────────────────────────────────────────────────────────────────────
//  Debug / UI label  (fixed-size to avoid heap allocation in component pool)
// ─────────────────────────────────────────────────────────────────────────────

struct NameComp
{
    char data[32]{};

    NameComp() = default;
    explicit NameComp(std::string_view s) noexcept
    {
        size_t n = std::min(s.size(), sizeof(data) - 1);
        std::memcpy(data, s.data(), n);
        data[n] = '\0';
    }
    [[nodiscard]] std::string_view view() const noexcept
    {
        return {data};
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Tags  (zero-size — EnTT stores them as a bitset, no per-entity memory)
// ─────────────────────────────────────────────────────────────────────────────

struct PlayerTag
{
};
struct EnemyTag
{
};
struct BossTag
{
};
struct DeadTag
{
}; // marked for removal on next resolve pass
struct ActiveTurnTag
{
}; // entity currently acting

} // namespace battle

#endif // BATTLE_COMPONENTS_H
