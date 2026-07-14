#ifndef BATTLE_MAPPER_H
#define BATTLE_MAPPER_H

#include "BattleTypes.h"

class Character;
class Enemy;

namespace BattleMapper
{
BattleActorState fromCharacter(const Character& player);
BattleActorState fromEnemy(const Enemy& enemy);
void applyPlayerHp(Character& player, int newHp);
void applyPlayerHp(Character& player, int newHp, int battleMaxHp);
}

#endif
