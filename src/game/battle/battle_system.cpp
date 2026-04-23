
#include "game/battle/battle_system.h"
#include "utils/logger.h"
#include <algorithm>
#include <cstdlib>

namespace battle {

static constexpr const char *ENEMIES_SCRIPT = "assets/scripts/game/enemies/enemies.lua";
static constexpr const char *CARDS_SCRIPT = "assets/scripts/game/cards/cards.lua";

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

BattleSystem::BattleSystem(entt::dispatcher &dispatcher, std::shared_ptr<AssetsInterface> assets,
                           std::shared_ptr<AudioInterface> audio)
    : m_dispatcher(dispatcher), m_assets(assets), m_audio(audio), m_turn(dispatcher, assets, audio)
{
    LOG_TRACE("[BattleSystem] Initialized");
}

bool BattleSystem::isActive() const noexcept
{
    return m_ctx.phase != CombatPhase::IDLE && m_ctx.phase != CombatPhase::VICTORY &&
           m_ctx.phase != CombatPhase::DEFEAT;
}

// ─────────────────────────────────────────────────────────────────────────────
//  ensureDbLoaded — load once from Lua, safe to call every startBattle
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::ensureDbLoaded()
{
    if (!m_enemy_db.isLoaded()) {
        m_enemy_db.load(ENEMIES_SCRIPT, m_assets);
        if (!m_enemy_db.isLoaded())
            LOG_WARN("[BattleSystem] Enemy database failed to load from {}", ENEMIES_SCRIPT);
        else
            LOG_INFO("[BattleSystem] Loaded {} enemy types", m_enemy_db.count());
    }

    if (m_card_db.count() == 0) {
        m_card_db.load(CARDS_SCRIPT, m_assets);
        if (m_card_db.count() == 0)
            LOG_WARN("[BattleSystem] Card database failed to load from {}", CARDS_SCRIPT);
        else
            LOG_INFO("[BattleSystem] Loaded {} card definitions", m_card_db.count());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  infoFor — look up EnemyInfo by entity's EnemyTypeComp
// ─────────────────────────────────────────────────────────────────────────────

const EnemyInfo *BattleSystem::infoFor(entt::entity enemy) const noexcept
{
    auto *et = m_reg.try_get<EnemyTypeComp>(enemy);
    if (!et)
        return nullptr;
    return m_enemy_db.findByType(et->type);
}

// ─────────────────────────────────────────────────────────────────────────────
//  startBattle
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::startBattle(int32_t level)
{
    LOG_INFO("[BattleSystem] Starting encounter — level {}", level);
    ensureDbLoaded();

    // Preserve player stats across level transitions
    HealthComp saved_hp{};
    DeckComp saved_deck{};
    HandComp saved_hand{};
    StrengthComp saved_str{};
    DexterityComp saved_dex{};
    bool has_player = (m_player != entt::null);

    if (has_player) {
        saved_hp = m_reg.get<HealthComp>(m_player);
        saved_deck = m_reg.get<DeckComp>(m_player);
        saved_hand = m_reg.get<HandComp>(m_player);
        saved_str = m_reg.get<StrengthComp>(m_player);
        saved_dex = m_reg.get<DexterityComp>(m_player);
    }

    m_reg.clear();
    m_ctx = {};
    m_player = entt::null;

    setupPlayer();

    if (has_player) {
        m_reg.replace<HealthComp>(m_player, saved_hp);
        m_reg.replace<DeckComp>(m_player, std::move(saved_deck));
        m_reg.replace<HandComp>(m_player, saved_hand);
        m_reg.replace<StrengthComp>(m_player, saved_str);
        m_reg.replace<DexterityComp>(m_player, saved_dex);
    }

    spawnEnemies(level);

    m_reg.emplace<LevelComp>(m_player, LevelComp{
                                           .current = level,
                                           .enemies_remaining = m_ctx.enemies_alive,
                                           .is_boss = (level == MAX_LEVELS),
                                       });
    m_ctx.player_entity = m_player;

    if (level == MAX_LEVELS) {
        m_dispatcher.enqueue<EvBossFightBegin>({level});
        transitionTo(CombatPhase::BOSS_INTRO);
    } else {
        transitionTo(CombatPhase::PLAYER_DRAW);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  setupPlayer
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::setupPlayer()
{
    m_player = m_reg.create();
    m_reg.emplace<PlayerTag>(m_player);
    m_reg.emplace<NameComp>(m_player, "Player");
    m_reg.emplace<HealthComp>(m_player, HealthComp{STARTER_HP, STARTER_HP});
    m_reg.emplace<BlockComp>(m_player);
    m_reg.emplace<EnergyComp>(m_player, EnergyComp{STARTER_ENERGY, STARTER_ENERGY});
    m_reg.emplace<StrengthComp>(m_player);
    m_reg.emplace<DexterityComp>(m_player);
    m_reg.emplace<StatusComp>(m_player);

    auto &deck = m_reg.emplace<DeckComp>(m_player);
    buildStarterDeck(deck);
    deck.shuffle();

    m_reg.emplace<HandComp>(m_player);
}

// ─────────────────────────────────────────────────────────────────────────────
//  spawnEnemies — select enemy types by level tier, spawn from database
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::spawnEnemies(int32_t level)
{
    static constexpr EnemyType POOL_LOW[] = {EnemyType::SKELETON, EnemyType::GOBLIN};
    static constexpr EnemyType POOL_MID[] = {EnemyType::TROLL, EnemyType::ARCHER};
    static constexpr EnemyType POOL_HIGH[] = {EnemyType::DARK_KNIGHT, EnemyType::NECROMANCER};

    int32_t count = (level == MAX_LEVELS) ? 1 : (rand() % 3) + 1;

    for (int32_t i = 0; i < count; ++i) {
        EnemyType type{};
        if (level == MAX_LEVELS) {
            type = (rand() % 2 == 0) ? EnemyType::BOSS_DRAGON : EnemyType::BOSS_LICH;
        } else if (level <= 3) {
            type = POOL_LOW[rand() % 2];
        } else if (level <= 6) {
            type = POOL_MID[rand() % 2];
        } else {
            type = POOL_HIGH[rand() % 2];
        }

        const EnemyInfo *info = m_enemy_db.findByType(type);
        if (!info) {
            LOG_WARN("[BattleSystem] No EnemyInfo for type {} — skipping", static_cast<int>(type));
            continue;
        }

        EnemyAI::spawn(m_reg, *info, level, i);
    }

    m_ctx.enemies_alive = count;
}

// ─────────────────────────────────────────────────────────────────────────────
//  buildStarterDeck
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::buildStarterDeck(DeckComp &deck) const
{
    auto push = [&](int id, int count = 1) {
        if (m_card_db.findByIndex(id))
            for (int i = 0; i < count; ++i)
                deck.draw_pile.push_back(id);
        else
            LOG_WARN("[BattleSystem] buildStarterDeck: card id {} not in db", id);
    };

    push(0, 4); // 4× Moonlight Strike
    push(1, 3); // 3× Shield Bash
    push(5, 2); // 2× Iron Wall
    push(2);    // 1× Lunar Heal
    push(6);    // 1× War Cry
}

// ─────────────────────────────────────────────────────────────────────────────
//  update
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::update(double dt)
{
    if (m_ctx.phase == CombatPhase::IDLE)
        return;
    m_ctx.phase_timer += static_cast<float>(dt);
    updatePhase(dt);
    m_dispatcher.update();
}

void BattleSystem::updatePhase(double dt)
{
    switch (m_ctx.phase) {
    case CombatPhase::SETUP:
        transitionTo(CombatPhase::PLAYER_DRAW);
        break;
    case CombatPhase::PLAYER_DRAW:
        phasePlayerDraw();
        break;
    case CombatPhase::PLAYER_TURN:
        phasePlayerTurn(dt);
        break;
    case CombatPhase::ENEMY_TURN:
        phaseEnemyTurn(dt);
        break;
    case CombatPhase::RESOLVE_DEATHS:
        phaseResolveDeaths();
        break;
    case CombatPhase::LEVEL_TRANSITION:
        phaseLevelTransition(dt);
        break;
    case CombatPhase::VICTORY:
        phaseVictory();
        break;
    case CombatPhase::DEFEAT:
        phaseDefeat();
        break;
    case CombatPhase::BOSS_INTRO:
        if (m_ctx.phase_timer >= BOSS_INTRO_DELAY)
            transitionTo(CombatPhase::PLAYER_DRAW);
        break;
    default:
        break;
    }
}

void BattleSystem::transitionTo(CombatPhase next)
{
    LOG_TRACE("[BattleSystem] Phase {} → {}", static_cast<int>(m_ctx.phase),
              static_cast<int>(next));
    m_ctx.phase = next;
    m_ctx.phase_timer = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Phase handlers
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::phasePlayerDraw()
{
    m_turn.beginPlayerTurn(m_reg, m_ctx);
    transitionTo(CombatPhase::PLAYER_TURN);
}

void BattleSystem::phasePlayerTurn(double /*dt*/)
{
    // Driven by player input via playerPlayCard / playerEndTurn
}

void BattleSystem::phaseEnemyTurn(double /*dt*/)
{
    // Pace each enemy action with a brief delay for readability
    if (m_ctx.waiting_for_anim) {
        if (m_ctx.phase_timer >= ENEMY_ACT_DELAY) {
            m_ctx.waiting_for_anim = false;
            m_ctx.phase_timer = 0.0f;
        }
        return;
    }

    if (!m_turn.hasMoreEnemies(m_ctx)) {
        m_turn.endEnemyTurns(m_reg);
        transitionTo(CombatPhase::RESOLVE_DEATHS);
        return;
    }

    // Step: apply start-of-turn effects, get acting entity
    entt::entity enemy = m_turn.beginEnemyTurn(m_reg, m_ctx);

    if (enemy != entt::null) {
        const EnemyInfo *info = infoFor(enemy);
        if (info) {
            // Execute stored intent, get raw damage
            int32_t raw = EnemyAI::executeIntent(m_reg, enemy, m_player, *info);
            if (raw > 0) {
                int32_t dmg = calcDamage(enemy, raw, m_player);
                applyDamage(m_player, dmg, enemy);
            }
        }
    }

    // Apply end-of-turn effects and advance turn index
    m_turn.endEnemyTurn(m_reg, m_ctx, enemy);

    // Pre-compute the next intent so the UI can display it before the next turn
    if (enemy != entt::null) {
        const EnemyInfo *info = infoFor(enemy);
        if (info) {
            EnemyAI::resolveIntent(m_reg, enemy, *info);
            m_dispatcher.enqueue<EvEnemyIntentChanged>({enemy});
        }
    }

    if (m_turn.hasMoreEnemies(m_ctx)) {
        m_ctx.waiting_for_anim = true;
        m_ctx.phase_timer = 0.0f;
    } else {
        m_turn.endEnemyTurns(m_reg);
        transitionTo(CombatPhase::RESOLVE_DEATHS);
    }
}

void BattleSystem::phaseResolveDeaths()
{
    std::vector<entt::entity> to_remove;
    for (auto e : m_reg.view<DeadTag, EnemyTag>())
        to_remove.push_back(e);

    for (auto e : to_remove) {
        m_dispatcher.enqueue<EvEntityDied>({e, false});
        m_reg.destroy(e);
        --m_ctx.enemies_alive;
    }

    if (m_player != entt::null && m_reg.all_of<DeadTag>(m_player)) {
        m_dispatcher.enqueue<EvEntityDied>({m_player, true});
        transitionTo(CombatPhase::DEFEAT);
        return;
    }

    if (m_ctx.enemies_alive <= 0) {
        transitionTo(CombatPhase::VICTORY);
        return;
    }

    if (m_resolving_card) {
        m_resolving_card = false;
        transitionTo(CombatPhase::PLAYER_TURN);
    } else {
        ++m_ctx.turn_number;
        transitionTo(CombatPhase::PLAYER_DRAW);
    }
}

void BattleSystem::phaseVictory()
{
    if (m_ctx.phase_timer <= static_cast<float>(1.0 / 60.0)) {
        m_dispatcher.enqueue<EvBattleVictory>();
        LOG_INFO("[BattleSystem] Victory!");
    }
    if (m_ctx.phase_timer >= 1.5f) {
        auto &lvl = m_reg.get<LevelComp>(m_player);
        if (lvl.current >= MAX_LEVELS) {
            LOG_INFO("[BattleSystem] All levels complete — game won!");
            transitionTo(CombatPhase::IDLE);
        } else {
            m_dispatcher.enqueue<EvLevelComplete>({lvl.current, lvl.current + 1});
            transitionTo(CombatPhase::LEVEL_TRANSITION);
        }
    }
}

void BattleSystem::phaseDefeat()
{
    if (m_ctx.phase_timer <= static_cast<float>(1.0 / 60.0)) {
        m_dispatcher.enqueue<EvBattleDefeat>();
        LOG_INFO("[BattleSystem] Defeat");
    }
}

void BattleSystem::phaseLevelTransition(double /*dt*/)
{
    if (m_ctx.phase_timer >= LEVEL_TRANSITION_DELAY) {
        auto &lvl = m_reg.get<LevelComp>(m_player);
        startBattle(lvl.current + 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Player input
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::playerPlayCard(int32_t hand_slot, entt::entity preferred_target)
{
    if (m_ctx.phase != CombatPhase::PLAYER_TURN)
        return;

    auto &hand = m_reg.get<HandComp>(m_player);
    auto &energy = m_reg.get<EnergyComp>(m_player);

    if (hand_slot < 0 || hand_slot >= hand.count)
        return;
    int32_t card_id = hand.slots[static_cast<size_t>(hand_slot)];
    const CardInfo *fx = m_card_db.findByIndex(card_id);
    if (!fx)
        return;

    if (energy.current < fx->cost) {
        this->m_audio->play_sfx("no_mana");
        LOG_TRACE("[BattleSystem] Not enough energy ({}/{}) for {}", energy.current, fx->cost,
                  fx->name);
        return;
    }
    energy.current -= fx->cost;

    // Use preferred_target if alive; otherwise fall back to first living enemy
    entt::entity target = entt::null;
    if (preferred_target != entt::null && m_reg.valid(preferred_target) &&
        !m_reg.all_of<DeadTag>(preferred_target)) {
        target = preferred_target;
    } else {
        for (auto e : m_reg.view<EnemyTag>(entt::exclude<DeadTag>)) {
            target = e;
            break;
        }
    }

    applyCardEffect(*fx, target);

    auto &deck = m_reg.get<DeckComp>(m_player);
    if (!fx->exhaust)
        deck.discard_pile.push_back(card_id);
    hand.removeAt(hand_slot);

    m_dispatcher.enqueue<EvCardPlayed>({m_player, card_id, target});
    m_resolving_card = true;
    transitionTo(CombatPhase::RESOLVE_DEATHS);
}

void BattleSystem::playerEndTurn()
{
    if (m_ctx.phase != CombatPhase::PLAYER_TURN)
        return;

    m_turn.endPlayerTurn(m_reg, m_ctx);

    // Refresh all enemy intents now that player's buffs/debuffs are resolved
    for (auto e : m_reg.view<EnemyTag>(entt::exclude<DeadTag>)) {
        const EnemyInfo *info = infoFor(e);
        if (info)
            EnemyAI::resolveIntent(m_reg, e, *info);
    }

    m_ctx.enemy_turn_index = 0;
    m_turn.buildEnemyOrder(m_reg);
    transitionTo(CombatPhase::ENEMY_TURN);
}

void BattleSystem::playerDiscard(int32_t hand_slot)
{
    if (m_ctx.phase != CombatPhase::PLAYER_TURN)
        return;

    auto &hand = m_reg.get<HandComp>(m_player);
    if (hand_slot < 0 || hand_slot >= hand.count)
        return;
    int32_t card_id = hand.slots[static_cast<size_t>(hand_slot)];
    if (card_id < 0)
        return;

    auto &deck = m_reg.get<DeckComp>(m_player);
    deck.discard_pile.push_back(card_id);
    hand.removeAt(hand_slot);
    m_dispatcher.enqueue<EvCardDiscarded>({m_player, card_id, hand_slot});
}

// ─────────────────────────────────────────────────────────────────────────────
//  Combat math
// ─────────────────────────────────────────────────────────────────────────────

int32_t BattleSystem::calcDamage(entt::entity attacker, int32_t base,
                                 entt::entity target) const noexcept
{
    int32_t dmg = base;
    if (auto *s = m_reg.try_get<StrengthComp>(attacker))
        dmg += s->value;
    if (auto *st = m_reg.try_get<StatusComp>(attacker))
        if (st->weak > 0)
            dmg = dmg * 3 / 4;
    if (auto *st = m_reg.try_get<StatusComp>(target))
        if (st->vulnerable > 0)
            dmg = dmg * 3 / 2;
    return std::max(0, dmg);
}

void BattleSystem::applyDamage(entt::entity target, int32_t amount, entt::entity source)
{
    if (target == entt::null)
        return;
    auto *hp = m_reg.try_get<HealthComp>(target);
    auto *bl = m_reg.try_get<BlockComp>(target);
    if (!hp)
        return;

    int32_t block_abs = bl ? std::min(bl->amount, amount) : 0;
    if (bl)
        bl->amount -= block_abs;
    int32_t actual = amount - block_abs;
    hp->current -= actual;

    m_dispatcher.enqueue<EvDamageDealt>({source, target, amount, actual, block_abs});
    if (hp->isDead())
        m_reg.emplace_or_replace<DeadTag>(target);
}

void BattleSystem::applyBlock(entt::entity target, int32_t amount)
{
    int32_t actual = amount;
    if (auto *dex = m_reg.try_get<DexterityComp>(target))
        actual += dex->value;
    if (auto *st = m_reg.try_get<StatusComp>(target))
        if (st->frail > 0)
            actual = actual * 3 / 4;
    actual = std::max(0, actual);
    if (auto *bl = m_reg.try_get<BlockComp>(target))
        bl->amount += actual;
    m_dispatcher.enqueue<EvBlockGained>({target, actual});
}

void BattleSystem::applyCardEffect(const CardInfo &fx, entt::entity target)
{
    if (fx.damage > 0 && target != entt::null) {
        int32_t dmg = calcDamage(m_player, fx.damage, target);
        applyDamage(target, dmg, m_player);
    }
    if (fx.block > 0) {
        applyBlock(m_player, fx.block);
        this->m_audio->play_sfx("block");
    }
    if (fx.heal > 0) {
        auto *hp = m_reg.try_get<HealthComp>(m_player);
        if (hp) {
            hp->current = std::min(hp->current + fx.heal, hp->max);
        }
        this->m_audio->play_sfx("heal");
    }
    if (fx.apply_strength != 0)
        m_reg.get<StrengthComp>(m_player).value += fx.apply_strength;
    if (target != entt::null) {
        if (auto *st = m_reg.try_get<StatusComp>(target)) {
            this->m_audio->play_sfx("skill");
            if (fx.apply_vulnerable)
                st->vulnerable = static_cast<int8_t>(
                    std::clamp<int32_t>(st->vulnerable + fx.apply_vulnerable, 0, 127));
            if (fx.apply_weak)
                st->weak =
                    static_cast<int8_t>(std::clamp<int32_t>(st->weak + fx.apply_weak, 0, 127));
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  shutdown
// ─────────────────────────────────────────────────────────────────────────────

void BattleSystem::shutdown()
{
    m_reg.clear();
    m_ctx = {};
    m_player = entt::null;
    LOG_TRACE("[BattleSystem] Shutdown");
}

} // namespace battle
