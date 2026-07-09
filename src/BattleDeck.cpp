#include "BattleDeck.h"

#include <algorithm>
#include <ctime>

BattleDeck::BattleDeck()
    : rng(static_cast<unsigned>(std::time(nullptr)))
{
    resetAndShuffle();
}

void BattleDeck::resetAndShuffle()
{
    cards.clear();

    // 这里重建的是“当前局中剩余卡组”，不是初始固定 5/3/2，
    // 因此永久削牌会持续影响后续每一次重洗。
    for (int i = 0; i < totalAttackCards; ++i) cards.push_back({CardType::Attack});
    for (int i = 0; i < totalDefenseCards; ++i) cards.push_back({CardType::Defense});
    for (int i = 0; i < totalHealCards; ++i) cards.push_back({CardType::Heal});

    std::shuffle(cards.begin(), cards.end(), rng);
}

std::vector<BattleCard> BattleDeck::drawCards(int count)
{
    // 牌堆不足时直接重建并洗牌，避免引入弃牌堆等更复杂的 TCG 机制。
    if (static_cast<int>(cards.size()) < count)
        resetAndShuffle();

    std::vector<BattleCard> hand;
    const int actualCount = std::min(count, static_cast<int>(cards.size()));
    for (int i = 0; i < actualCount; ++i)
    {
        hand.push_back(cards.back());
        cards.pop_back();
    }
    return hand;
}

void BattleDeck::permanentlyRemoveAttackCard()
{
    // 下限保护避免被重复削到负数。
    if (totalAttackCards > 0)
        --totalAttackCards;
}

void BattleDeck::permanentlyRemoveDefenseCard()
{
    if (totalDefenseCards > 0)
        --totalDefenseCards;
}
