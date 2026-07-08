// EnemyFactory.cpp
#include "EnemyFactory.h"
#include "SimpleJson.h"
#include <stdexcept>

#include <cstdlib>
#include <iostream>

EnemyFactory::EnemyFactory()
{
}

void EnemyFactory::loadFromJson(const std::string& filepath)
{
    templates.clear();

    try
    {
        JsonValue root = parseJsonFile(filepath);

        // root should be an array of enemy objects
        if (root.type != JsonValue::Array)
        {
            std::cout << "[EnemyFactory] Warning: JSON root is not an array.\n";
            return;
        }

        for (size_t i = 0; i < root.size(); ++i)
        {
            const JsonValue& obj = root[i];

            std::string name  = obj.has("name")   ? obj["name"].asString()   : "Unknown";
            int hp            = obj.has("hp")     ? obj["hp"].asInt()        : 30;
            int attack        = obj.has("attack") ? obj["attack"].asInt()    : 5;
            int defense       = obj.has("defense")? obj["defense"].asInt()   : 1;
            int spd           = obj.has("spd")    ? obj["spd"].asInt()       : 3;
            int exp           = obj.has("exp")    ? obj["exp"].asInt()        : 20;
            int gold          = obj.has("gold")   ? obj["gold"].asInt()       : 10;

            // Parse tier
            EnemyTier tier = EnemyTier::Minion;
            if (obj.has("tier"))
            {
                std::string tierStr = obj["tier"].asString();
                if (tierStr == "Elite" || tierStr == "elite") tier = EnemyTier::Elite;
                else if (tierStr == "Boss" || tierStr == "boss") tier = EnemyTier::Boss;
            }

            // Parse loot array
            std::vector<std::string> loot;
            if (obj.has("loot") && obj["loot"].type == JsonValue::Array)
            {
                const JsonValue& lootArr = obj["loot"];
                for (size_t j = 0; j < lootArr.size(); ++j)
                {
                    loot.push_back(lootArr[j].asString());
                }
            }

            templates.emplace_back(name, hp, attack, defense, spd, tier, exp, gold, loot);
        }

        std::cout << "[EnemyFactory] Loaded " << templates.size() << " enemy templates.\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "[EnemyFactory] Error loading JSON: " << e.what() << "\n";
    }
}

std::vector<Enemy> EnemyFactory::generateEnemies(int stage) const
{
    std::vector<Enemy> result;

    if (templates.empty())
    {
        // Fallback: generate default enemy
        Enemy fallback("\xe5\x8f\xb2\xe8\x8e\xb1\xe5\xa7\x86", 45 * stage, 12 * stage, 3 * stage, 60 * stage, 30 * stage);
        result.push_back(fallback);
        return result;
    }

    auto pick = [this](EnemyTier tier) -> Enemy {
        const Enemy& tmpl = pickRandomTemplate();
        Enemy e(
            tmpl.getName(),
            tmpl.getHp(),
            tmpl.getAttack(),
            tmpl.getDefense(),
            tmpl.getSpd(),
            tier,
            tmpl.getRewardExp(),
            tmpl.getRewardGold(),
            tmpl.getLootItems()
        );
        e.applyTierMultiplier();
        return e;
    };

    switch (stage)
    {
    case 1:
        // 1 Minion
        result.push_back(pick(EnemyTier::Minion));
        break;

    case 2:
        // 2 Minions
        result.push_back(pick(EnemyTier::Minion));
        result.push_back(pick(EnemyTier::Minion));
        break;

    case 3:
        // 1 Minion + 1 Elite
        result.push_back(pick(EnemyTier::Minion));
        result.push_back(pick(EnemyTier::Elite));
        break;

    case 4:
        // 2 Elites
        result.push_back(pick(EnemyTier::Elite));
        result.push_back(pick(EnemyTier::Elite));
        break;

    case 5:
        // 1 Boss
        result.push_back(pick(EnemyTier::Boss));
        break;

    default:
        // For stages beyond 5, scale up
        {
            int count = (stage <= 6) ? 2 : 3;
            EnemyTier t = (stage >= 7) ? EnemyTier::Boss : EnemyTier::Elite;
            for (int i = 0; i < count; ++i)
                result.push_back(pick(t));
        }
        break;
    }

    return result;
}

const Enemy& EnemyFactory::getTemplate(int index) const
{
    if (index < 0 || index >= static_cast<int>(templates.size()))
        throw std::out_of_range("EnemyFactory::getTemplate index out of range");
    return templates[index];
}

const Enemy& EnemyFactory::pickRandomTemplate() const
{
    if (templates.empty())
    {
        static Enemy fallback;
        return fallback;
    }
    int idx = rand() % templates.size();
    return templates[idx];
}
