
#include "game/battle/turn_manager.h"
#include "utils/logger.h"
#include <algorithm>

namespace battle {

TurnManager::TurnManager(entt::dispatcher &dispatcher, std::shared_ptr<AssetsInterface> m_assets,
                         std::shared_ptr<AudioInterface> m_audio)
    : m_dispatcher(dispatcher), m_assets(m_assets), m_audio(m_audio)
{
    int channels, sample_rate;
    short *data;
    auto sample = this->m_assets->loadAudio("assets/audio/card/card-new-sfx.ogg", channels,
                                            sample_rate, data);
    this->m_audio->load("card_new_1", channels, sample, sample_rate, data);
}

TurnManager::~TurnManager()
{
    this->m_audio->unload("card_new_1");
}

// ─────────────────────────────────────────────────────────────────────────────
//  beginPlayerTurn
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::beginPlayerTurn(entt::registry &reg, CombatContext &ctx)
{
    entt::entity player = ctx.player_entity;
    applyStartOfTurn(reg, player);

    auto &hand = reg.get<HandComp>(player);
    auto &deck = reg.get<DeckComp>(player);

    if (hand.isEmpty()) {
        // First turn of the battle: fill hand to capacity
        while (!hand.isFull()) {
            int32_t card = drawCard(reg, player);
            if (card < 0)
                break;
            m_dispatcher.enqueue<EvCardDrawn>({player, card, hand.count - 1});
        }
        this->m_audio->play_sfx("card_new_1");
        LOG_TRACE("[TurnManager] Initial draw: {} cards", hand.count);
    } else {
        // Subsequent turns: if hand full → randomly discard 1, then draw 1
        if (hand.count >= HAND_CAPACITY) {
            int32_t idx = rand() % hand.count;
            int32_t cid = hand.slots[static_cast<size_t>(idx)];
            if (cid >= 0) {
                deck.discard_pile.push_back(cid);
                m_dispatcher.enqueue<EvCardDiscarded>({player, cid, idx});
            }
            hand.removeAt(idx);
            LOG_TRACE("[TurnManager] Hand full — randomly discarded slot {} (card {})", idx, cid);
        }
        int32_t card = drawCard(reg, player);
        if (card >= 0) {
            m_dispatcher.enqueue<EvCardDrawn>({player, card, hand.count - 1});
            this->m_audio->play_sfx("card_new_1");
        }
    }

    m_dispatcher.enqueue<EvTurnStarted>({player, true});
    LOG_TRACE("[TurnManager] Player turn began (turn {})", ctx.turn_number);
}

// ─────────────────────────────────────────────────────────────────────────────
//  endPlayerTurn
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::endPlayerTurn(entt::registry &reg, CombatContext &ctx)
{
    entt::entity player = ctx.player_entity;
    // Cards are kept for the next turn — no discard here
    applyEndOfTurn(reg, player);
    m_dispatcher.enqueue<EvTurnEnded>({player, true});
    auto &hand = reg.get<HandComp>(player);
    LOG_TRACE("[TurnManager] Player turn ended (hand={}/{})", hand.count, HAND_CAPACITY);
}

// ─────────────────────────────────────────────────────────────────────────────
//  buildEnemyOrder
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::buildEnemyOrder(entt::registry &reg)
{
    m_enemy_order.clear();
    auto view = reg.view<EnemyTag, TurnOrderComp>(entt::exclude<DeadTag>);
    for (auto e : view)
        m_enemy_order.push_back(e);

    std::sort(m_enemy_order.begin(), m_enemy_order.end(), [&](entt::entity a, entt::entity b) {
        return reg.get<TurnOrderComp>(a).priority < reg.get<TurnOrderComp>(b).priority;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  beginEnemyTurn — apply start effects, fire event, return acting entity
// ─────────────────────────────────────────────────────────────────────────────

entt::entity TurnManager::beginEnemyTurn(entt::registry &reg, CombatContext &ctx)
{
    while (ctx.enemy_turn_index < static_cast<int32_t>(m_enemy_order.size())) {
        entt::entity enemy = m_enemy_order[static_cast<size_t>(ctx.enemy_turn_index)];

        // Skip enemies that died before their turn (e.g., poison)
        if (reg.all_of<DeadTag>(enemy)) {
            ++ctx.enemy_turn_index;
            continue;
        }

        applyStartOfTurn(reg, enemy);
        m_dispatcher.enqueue<EvTurnStarted>({enemy, false});
        return enemy;
    }
    return entt::null;
}

// ─────────────────────────────────────────────────────────────────────────────
//  endEnemyTurn — apply end effects, fire event, advance index
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::endEnemyTurn(entt::registry &reg, CombatContext &ctx, entt::entity enemy)
{
    if (enemy != entt::null)
        applyEndOfTurn(reg, enemy);

    m_dispatcher.enqueue<EvTurnEnded>({enemy, false});
    ++ctx.enemy_turn_index;
}

// ─────────────────────────────────────────────────────────────────────────────
//  endEnemyTurns
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::endEnemyTurns(entt::registry & /*reg*/)
{
    m_enemy_order.clear();
    LOG_TRACE("[TurnManager] All enemy turns complete");
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCard
// ─────────────────────────────────────────────────────────────────────────────

int32_t TurnManager::drawCard(entt::registry &reg, entt::entity player)
{
    auto &deck = reg.get<DeckComp>(player);
    auto &hand = reg.get<HandComp>(player);

    if (hand.isFull()) {
        LOG_TRACE("[TurnManager] Hand full, skipping draw");
        return -1;
    }

    if (deck.draw_pile.empty()) {
        if (deck.discard_pile.empty()) {
            LOG_TRACE("[TurnManager] Deck and discard both empty");
            return -1;
        }
        LOG_TRACE("[TurnManager] Reshuffling discard into draw pile");
        deck.refillFromDiscard();
    }

    int32_t card_id = deck.draw_pile.back();
    deck.draw_pile.pop_back();
    hand.add(card_id);

    LOG_TRACE("[TurnManager] Drew card {} (hand count={})", card_id, hand.count);
    return card_id;
}

// ─────────────────────────────────────────────────────────────────────────────
//  applyStartOfTurn
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::applyStartOfTurn(entt::registry &reg, entt::entity entity)
{
    if (auto *e = reg.try_get<EnergyComp>(entity))
        e->current = e->max;

    if (auto *b = reg.try_get<BlockComp>(entity))
        b->amount = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  applyEndOfTurn
// ─────────────────────────────────────────────────────────────────────────────

void TurnManager::applyEndOfTurn(entt::registry &reg, entt::entity entity)
{
    auto *st = reg.try_get<StatusComp>(entity);
    if (!st)
        return;

    auto &hp = reg.get<HealthComp>(entity);

    if (st->poison > 0) {
        hp.current -= st->poison;
        --st->poison;
        if (hp.isDead())
            reg.emplace_or_replace<DeadTag>(entity);
    }

    if (st->burn > 0) {
        hp.current -= st->burn;
        if (hp.isDead())
            reg.emplace_or_replace<DeadTag>(entity);
    }

    if (st->regen > 0)
        hp.current = std::min(hp.current + static_cast<int32_t>(st->regen), hp.max);

    if (st->vulnerable > 0)
        --st->vulnerable;
    if (st->weak > 0)
        --st->weak;
    if (st->frail > 0)
        --st->frail;
}

} // namespace battle
