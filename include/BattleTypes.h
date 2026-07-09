#ifndef BATTLE_TYPES_H
#define BATTLE_TYPES_H

#include <string>
#include <vector>

// 战斗结果会向上层菜单、冒险系统和任务系统传播，
// 因此保持语义简单稳定，只区分胜利、逃跑、死亡三种结局。
enum class BattleOutcome
{
    PLAYER_WIN,
    PLAYER_ESCAPE,
    PLAYER_DEAD
};

// BattleAction 表示“本回合最终执行的动作”，
// CardType 则表示“手牌里抽到的牌面类型”。
// 二者大部分时候一一对应，但指令系统会绕过手牌直接指定动作。
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

enum class ElementType
{
    Neutral,
    Fire,
    Water,
    Grass,
    Mech
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
    ElementType element = ElementType::Neutral;
    double incomingDamageMultiplier = 1.0;
    int vulnerabilityStacks = 0;
    // disarmed 表示“当前回合无法攻击”。
    // pendingDisarm 由防反在结算阶段设置，在下一回合开始时转成 disarmed。
    bool disarmed = false;
    bool pendingDisarm = false;
    bool commandUsed = false;

    bool isAlive() const { return currentHp > 0; }
};

// BattleTurnContext 只保存“当前回合”的公开信息与临时手牌，
// 不承载跨回合状态，避免和 BattleActorState 的职责混在一起。
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
std::string toString(ElementType element);
bool hasTypeAdvantage(ElementType attacker, ElementType defender);

#endif
