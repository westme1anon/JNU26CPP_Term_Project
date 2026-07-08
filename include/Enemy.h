// Enemy.h
#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include <vector>

// ============================================================
// EnemyTier
// ------------------------------------------------------------
// 敌人分级枚举。
// ============================================================

enum class EnemyTier
{
    Minion,  // 普通怪（属性正常）
    Elite,   // 精英怪（属性 ×1.5）
    Boss     // 首领（属性 ×3.0）
};

// ============================================================
// Enemy
// ------------------------------------------------------------
// 敌人类。
// 表示战斗中的怪物或对手，包含生命、攻击、防御、速度、
// 分级、携带战利品等信息。
// ============================================================

class Enemy
{
private:
    std::string name;
    int hp;
    int attack;
    int defense;
    int spd;
    EnemyTier tier;
    int rewardExp;
    int rewardGold;
    std::vector<std::string> lootItems;

public:
    Enemy();
    Enemy(
        const std::string& name,
        int hp,
        int attack,
        int defense,
        int rewardExp,
        int rewardGold
    );
    Enemy(
        const std::string& name,
        int hp,
        int attack,
        int defense,
        int spd,
        EnemyTier tier,
        int rewardExp,
        int rewardGold,
        const std::vector<std::string>& lootItems
    );

    // 显示敌人信息。
    void showInfo() const;

    // 敌人受到伤害。
    void takeDamage(int damage);

    // 判断敌人是否存活。
    bool isAlive() const;

    // 根据分级应用属性倍率（Minion=1.0, Elite=1.5, Boss=3.0）。
    void applyTierMultiplier();

    // 添加战利品物品名。
    void addLootItem(const std::string& item);

    std::string getName() const;
    int getHp() const;
    int getAttack() const;
    int getDefense() const;
    int getSpd() const;
    EnemyTier getTier() const;
    int getRewardExp() const;
    int getRewardGold() const;
    const std::vector<std::string>& getLootItems() const;
};

#endif
