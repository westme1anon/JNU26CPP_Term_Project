#ifndef BATTLE_MAPPER_H
#define BATTLE_MAPPER_H

#include "BattleTypes.h"

class Character;
class Enemy;

namespace BattleMapper
{
// 把永久角色属性映射成“本场战斗快照”。
// 后续临时状态只修改 BattleActorState，不直接污染 Character / Enemy。
BattleActorState fromCharacter(const Character& player);
BattleActorState fromEnemy(const Enemy& enemy);

// 战斗过程中只需要回写玩家生命值；其余奖励由 BattleSystem 外层处理。
void applyPlayerHp(Character& player, int newHp);
ElementType inferElementFromName(const std::string& name);
}

#endif
