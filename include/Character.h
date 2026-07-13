#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>

// ============================================================
// Character
// ------------------------------------------------------------
// 玩家角色类。
// 负责维护角色基础属性、等级成长、金币经验变化、存档序列化等。
// ============================================================

class Character
{
private:
    std::string name;
    int level;
    int hp;
    int maxHp;
    int exp;
    int gold;
    int attack;
    int defense;
    bool invincible;

public:
    Character();

    // 创建新角色，并初始化默认属性。
    void create(const std::string& characterName);

    // 显示角色完整信息。
    void showInfo() const;

    // 获得经验值，并自动检测是否升级。
    void gainExp(int amount);

    // 获得金币。
    void gainGold(int amount);

    // 消耗金币。
    // 返回 true 表示金币足够且扣除成功。
    bool costGold(int amount);

    // 角色受到伤害。
    // 实际扣血量可结合防御力计算。
    void takeDamage(int damage);

    // 恢复生命值，但不超过 maxHp。
    void heal(int amount);

    // 直接设置等级。
    void setLevel(int newLevel);

    // 直接设置最大生命值。
    void setMaxHp(int newMaxHp);

    // 将所有属性最大化。
    void maxStats();

    // 判断角色是否存活。
    bool isAlive() const;

    // 无敌状态。
    bool isInvincible() const;
    void setInvincible(bool value);

    // 检查当前经验是否满足升级条件。
    // 若满足则提升等级并增长属性。
    void levelUpIfNeeded();

    // 增加攻击力。
    void addAttack(int value);

    // 增加防御力。
    void addDefense(int value);

    // 将角色数据序列化为字符串，用于保存到文件。
    std::string serialize() const;

    // 从字符串恢复角色数据。
    void deserialize(const std::string& data);

    // Getter
    std::string getName() const;
    int getLevel() const;
    int getHp() const;
    int getMaxHp() const;
    int getExp() const;
    int getGold() const;
    int getAttack() const;
    int getDefense() const;
};

#endif