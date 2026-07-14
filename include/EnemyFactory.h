// EnemyFactory.h
#ifndef ENEMY_FACTORY_H
#define ENEMY_FACTORY_H

#include "Enemy.h"

#include <string>
#include <vector>

// ============================================================
// EnemyFactory
// ------------------------------------------------------------
// 敌人生产工厂。
// 从 JSON 加载敌人模板，根据冒险阶段动态生成敌人配置。
//
// 阶段映射：
//   第1场 → 1 Minion
//   第2场 → 2 Minions
//   第3场 → 1 Minion + 1 Elite
//   第4场 → 2 Elites
//   第5场 → 1 Boss
// ============================================================

class EnemyFactory
{
private:
    std::vector<Enemy> templates;  // 从 JSON 加载的敌人模板

public:
    EnemyFactory();

    // 从 JSON 文件加载敌人模板。
    void loadFromJson(const std::string& filepath);

    // 根据冒险阶段（1-based）生成敌人列表。
    std::vector<Enemy> generateEnemies(int stage) const;

    // 获取模板数量。
    int templateCount() const { return static_cast<int>(templates.size()); }

    // 随机选取一个模板。
    // 获取指定下标的模板。
    const Enemy& getTemplate(int index) const;

    // 随机选取一个模板。
    const Enemy& pickRandomTemplate() const;

    // 按层级随机选取模板；若该层级没有模板，则回退到任意模板。
    const Enemy& pickRandomTemplate(EnemyTier tier) const;
};

#endif
