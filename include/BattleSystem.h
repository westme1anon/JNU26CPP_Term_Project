#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "BattleAI.h"
#include "BattleDeck.h"
#include "BattleMapper.h"
#include "BattleTypes.h"
#include "Character.h"
#include "Enemy.h"
#include "EnemyFactory.h"
#include "Inventory.h"

#include <string>
#include <vector>

struct BattleResult
{
    BattleOutcome outcome = BattleOutcome::PLAYER_WIN;
    std::string enemyName;
};

// BattleSystem 负责把 Character / Enemy 映射成独立的战斗态，
// 再用卡牌状态机完成单场战斗。
class BattleSystem
{
private:
    EnemyFactory enemyFactory;
    static constexpr int kHandSize = 3;

public:
    BattleSystem();

    void loadEnemies();
    EnemyFactory& getEnemyFactory() { return enemyFactory; }
    void showEnemyList() const;
    Enemy selectEnemy(int index) const;
    int enemyCount() const;

    BattleResult startBattle(Character& player, Inventory& inventory);
    BattleResult startBattle(Character& player, Inventory& inventory, int stage, bool isBoss);
    BattleResult startBattle(Character& player, Inventory& inventory, const std::vector<Enemy>& enemies, bool isBoss);

private:
    BattleOutcome fightOneEnemy(Character& player, Inventory& inventory, const Enemy& enemy) const;
    BattleOutcome runCardBattle(Character& player, Inventory& inventory, const Enemy& enemy) const;
    void beginRound(BattleActorState& playerState, BattleActorState& enemyState) const;
    void printBattleState(
        const BattleActorState& playerState,
        const BattleActorState& enemyState,
        int round,
        bool playerFirst) const;
    void printHand(const std::vector<BattleCard>& hand, const std::string& ownerLabel) const;
    BattleDecision getPlayerDecision(
        const BattleTurnContext& context,
        const BattleActorState& playerState,
        const BattleActorState& enemyState,
        const Inventory& inventory) const;
    BattleDecision chooseCommandAction(
        const BattleActorState& playerState,
        const BattleActorState& enemyState,
        const Inventory& inventory) const;
    void applyDefenseResolution(
        BattleActorState& actor,
        BattleDeck& actorDeck,
        BattleAction actorAction,
        BattleActorState& target,
        BattleDeck& targetDeck,
        BattleAction targetAction) const;
    void executeAction(
        BattleActorState& actor,
        BattleActorState& target,
        const BattleDecision& decision,
        Inventory* inventory) const;
    int calculateDamage(const BattleActorState& attacker, const BattleActorState& defender) const;
    int calculateHealAmount(const BattleActorState& actor) const;
    std::vector<int> collectBattleUsableItems(const Inventory& inventory) const;
    bool determinePlayerFirst(const BattleActorState& playerState, const BattleActorState& enemyState) const;
    std::string makeHpBar(int hp, int maxHp, int width = 24) const;
};

#endif
