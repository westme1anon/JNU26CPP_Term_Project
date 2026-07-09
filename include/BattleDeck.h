#ifndef BATTLE_DECK_H
#define BATTLE_DECK_H

#include "BattleTypes.h"

#include <random>
#include <vector>

class BattleDeck
{
private:
    std::vector<BattleCard> cards;
    // 这三个计数代表“当前剩余可重建的卡组配方”，
    // 防反造成的永久削牌会直接修改这里，而不是只删掉当前抽牌堆里的某一张。
    int totalAttackCards = 5;
    int totalDefenseCards = 3;
    int totalHealCards = 2;
    mutable std::mt19937 rng;

    // 当抽牌堆不足时，按当前配方重建整副牌再洗牌。
    void resetAndShuffle();

public:
    BattleDeck();

    // 每回合默认抽 3 张；如果牌不够，会先按“削牌后的配方”重洗。
    std::vector<BattleCard> drawCards(int count = 3);
    void permanentlyRemoveAttackCard();
    void permanentlyRemoveDefenseCard();
};

#endif
