#ifndef BATTLE_TYPES_H
#define BATTLE_TYPES_H

#include <string>
#include <vector>

enum class BattleOutcome
{
    PLAYER_WIN,
    PLAYER_ESCAPE,
    PLAYER_DEAD
};

enum class BattleAction
{
    None = 0,
    Attack = 1,
    Defense = 2,
    Heal = 3,
    Escape = 4,
    UseItem = 5
};

enum class CardType
{
    Attack = 1,
    Defense = 2,
    Heal = 3
};

struct BattleCard
{
    CardType type = CardType::Attack;

    std::string getName() const;
    int getActionId() const;
};

struct BattleDecision
{
    BattleAction action = BattleAction::None;
    bool usedCommand = false;
    bool usedBattleItem = false;
    int selectedIndex = -1;
    int inventoryIndex = -1;
};

struct BattleActorState
{
    std::string name;
    int currentHp = 0;
    int maxHp = 0;
    int attack = 0;
    int defense = 0;
    int speed = 0;
    double incomingDamageMultiplier = 1.0;
    int vulnerabilityStacks = 0;
    bool disarmed = false;
    bool pendingDisarm = false;
    bool commandUsed = false;

    bool isAlive() const { return currentHp > 0; }
};

struct BattleTurnContext
{
    int round = 1;
    bool playerFirst = true;
    BattleAction revealedPlayerAction = BattleAction::None;
    BattleAction revealedEnemyAction = BattleAction::None;
    std::vector<BattleCard> playerHand;
    std::vector<BattleCard> enemyHand;
};

std::string toString(BattleAction action);
std::string toString(CardType type);

#endif
