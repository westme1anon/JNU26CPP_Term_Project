#include "BattleMapper.h"
#include "Character.h"
#include "Enemy.h"

#include <algorithm>
#include <cctype>

namespace
{
bool containsAny(const std::string& text, const std::initializer_list<const char*>& tokens)
{
    for (const char* token : tokens)
    {
        if (text.find(token) != std::string::npos)
            return true;
    }
    return false;
}

std::string toLowerAscii(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}
}

namespace BattleMapper
{
BattleActorState fromCharacter(const Character& player)
{
    BattleActorState state;
    state.name = player.getName();
    state.currentHp = player.getHp();
    state.maxHp = player.getMaxHp();
    state.attack = player.getAttack();
    state.defense = player.getDefense();

    // 角色没有独立 speed 字段，先用等级和攻击力构造一个稳定速度值，
    // 让现有成长系统仍然能影响卡牌战斗里的先后手。
    state.speed = std::max(1, player.getLevel() + player.getAttack() / 5);
    state.element = inferElementFromName(player.getName());
    return state;
}

BattleActorState fromEnemy(const Enemy& enemy)
{
    BattleActorState state;
    state.name = enemy.getName();
    // 旧项目里的敌人数值是按传统回合战斗设计的；
    // 直接照搬到卡牌版的 ATK-DEF 公式里，会让高防 Boss 只吃 1 点伤害。
    // 这里把原始值压缩到更适合卡牌对战的区间，同时保留 tier 的强弱层次。
    int mappedHp = enemy.getHp();
    int mappedAttack = enemy.getAttack();
    int mappedDefense = enemy.getDefense();

    switch (enemy.getTier())
    {
    case EnemyTier::Minion:
        mappedHp = std::max(35, enemy.getHp() * 4 / 5);
        mappedAttack = std::max(8, enemy.getAttack() * 9 / 10);
        mappedDefense = std::max(1, enemy.getDefense() * 2 / 3);
        break;

    case EnemyTier::Elite:
        mappedHp = std::max(55, enemy.getHp() * 3 / 4);
        mappedAttack = std::max(12, enemy.getAttack() * 17 / 20);
        mappedDefense = std::max(2, enemy.getDefense() * 3 / 5);
        break;

    case EnemyTier::Boss:
        mappedHp = std::max(90, enemy.getHp() * 2 / 3);
        mappedAttack = std::max(16, enemy.getAttack() * 4 / 5);
        mappedDefense = std::max(3, enemy.getDefense() / 2);
        break;
    }

    // 防御再做一次软上限，避免在卡牌战斗里出现“双方只能互相刮痧”的极端情况。
    mappedDefense = std::min(mappedDefense, std::max(3, mappedAttack / 2));

    state.currentHp = mappedHp;
    state.maxHp = mappedHp;
    state.attack = mappedAttack;
    state.defense = mappedDefense;
    state.speed = enemy.getSpd();
    state.element = inferElementFromName(enemy.getName());
    return state;
}

void applyPlayerHp(Character& player, int newHp)
{
    // 只通过 Character 现有接口回写生命值，避免直接破坏角色封装。
    newHp = std::max(0, std::min(newHp, player.getMaxHp()));
    const int currentHp = player.getHp();
    if (newHp < currentHp)
        player.takeDamage(currentHp - newHp);
    else if (newHp > currentHp)
        player.heal(newHp - currentHp);
}

ElementType inferElementFromName(const std::string& name)
{
    // 当前项目没有显式元素字段，先做弱推断。
    // 后续如果角色/敌人配置里补了 element，这里可以直接替换成正式映射。
    const std::string lowered = toLowerAscii(name);
    if (containsAny(lowered, {"fire", "flame", "dragon"})) return ElementType::Fire;
    if (containsAny(lowered, {"water", "ice", "sea", "river"})) return ElementType::Water;
    if (containsAny(lowered, {"grass", "leaf", "tree", "forest"})) return ElementType::Grass;
    if (containsAny(lowered, {"mech", "metal", "iron", "knight"})) return ElementType::Mech;
    return ElementType::Neutral;
}
}
