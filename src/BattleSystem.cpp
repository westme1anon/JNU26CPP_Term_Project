#include "BattleSystem.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "PathUtil.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

BattleSystem::BattleSystem()
{
    loadEnemies();
}

void BattleSystem::loadEnemies()
{
    std::string resolved = PathUtil::resolvePath(GameConfig::ENEMIES_FILE_PATH);
    if (resolved.empty()) resolved = GameConfig::ENEMIES_FILE_PATH;
    enemyFactory.loadFromJson(resolved);
}

void BattleSystem::showEnemyList() const
{
    ConsoleUI::printTitle("可挑战敌人");

    const int count = enemyFactory.templateCount();
    if (count == 0)
    {
        std::cout << "没有可用的敌人模板。\n";
        return;
    }

    const Enemy& sample = enemyFactory.pickRandomTemplate();
    std::cout << "共 " << count << " 种敌人模板\n";
    std::cout << "示例: ";
    sample.showInfo();
}

Enemy BattleSystem::selectEnemy(int index) const
{
    const int count = enemyFactory.templateCount();
    if (count == 0 || index < 0 || index >= count)
    {
        const Enemy& tmpl = enemyFactory.pickRandomTemplate();
        Enemy fallback(
            tmpl.getName(),
            tmpl.getHp(),
            tmpl.getAttack(),
            tmpl.getDefense(),
            tmpl.getSpd(),
            EnemyTier::Minion,
            tmpl.getRewardExp(),
            tmpl.getRewardGold(),
            tmpl.getLootItems());
        fallback.applyTierMultiplier();
        return fallback;
    }

    const Enemy& tmpl = enemyFactory.getTemplate(index);
    Enemy enemy(
        tmpl.getName(),
        tmpl.getHp(),
        tmpl.getAttack(),
        tmpl.getDefense(),
        tmpl.getSpd(),
        EnemyTier::Minion,
        tmpl.getRewardExp(),
        tmpl.getRewardGold(),
        tmpl.getLootItems());
    enemy.applyTierMultiplier();
    return enemy;
}

int BattleSystem::enemyCount() const
{
    return enemyFactory.templateCount();
}

BattleResult BattleSystem::startBattle(Character& player, Inventory& inventory)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("自由战斗");

    if (enemyFactory.templateCount() == 0)
    {
        std::cout << "没有可用的敌人。\n";
        ConsoleUI::pause();
        return result;
    }

    // 自由战斗采用随机敌人；Adventure 模式则由上层传入固定敌人序列。
    Enemy enemy = selectEnemy(-1);
    result.enemyName = enemy.getName();
    result.outcome = fightOneEnemy(player, inventory, enemy);

    if (result.outcome == BattleOutcome::PLAYER_WIN)
    {
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "战斗胜利！你击败了 " << enemy.getName() << "。\n";
        ConsoleUI::resetColor();
        player.gainExp(enemy.getRewardExp());
        player.gainGold(enemy.getRewardGold());
    }
    else if (result.outcome == BattleOutcome::PLAYER_ESCAPE)
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "你脱离了战斗。\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "战斗失败。" << player.getName() << " 被 " << enemy.getName() << " 击倒了。\n";
        ConsoleUI::resetColor();
    }

    ConsoleUI::pause();
    return result;
}

BattleResult BattleSystem::startBattle(Character& player, Inventory& inventory, int stage, bool isBoss)
{
    std::vector<Enemy> enemies = enemyFactory.generateEnemies(stage);
    return startBattle(player, inventory, enemies, isBoss);
}

BattleResult BattleSystem::startBattle(Character& player, Inventory& inventory, const std::vector<Enemy>& enemies, bool isBoss)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    // 这个重载专门给 AdventureManager 使用，确保“展示的敌人列表”和“实际交战对象”一致。
    if (enemies.empty())
    {
        std::cout << "[BattleSystem] Error: No enemies available.\n";
        return result;
    }

    ConsoleUI::printLine('-');
    ConsoleUI::setColor(isBoss ? GameConfig::COLOR_WARNING : GameConfig::COLOR_TITLE);
    std::cout << "遭遇 " << enemies.size() << " 个敌人：\n";
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        std::cout << "  " << (i + 1) << ". ";
        enemies[i].showInfo();
    }
    ConsoleUI::resetColor();

    for (const auto& enemy : enemies)
    {
        if (!result.enemyName.empty())
            result.enemyName += ", ";
        result.enemyName += enemy.getName();
    }

    int defeated = 0;
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        // 冒险模式仍保持“连续单挑”的结构，
        // 只共享玩家当前生命值，不扩展成一个多目标公共牌局。
        if (i > 0)
        {
            ConsoleUI::clearScreen();
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "下一个敌人！剩余 " << (enemies.size() - i) << " 个...\n";
            ConsoleUI::resetColor();
            ConsoleUI::pause();
        }

        const BattleOutcome fightOutcome = fightOneEnemy(player, inventory, enemies[i]);
        if (fightOutcome == BattleOutcome::PLAYER_WIN)
        {
            ++defeated;
            player.gainExp(enemies[i].getRewardExp());
            player.gainGold(enemies[i].getRewardGold());
            continue;
        }

        if (fightOutcome == BattleOutcome::PLAYER_ESCAPE)
        {
            result.outcome = BattleOutcome::PLAYER_ESCAPE;
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "你从 " << enemies[i].getName() << " 面前脱离了。\n";
            ConsoleUI::resetColor();
            ConsoleUI::pause();
            return result;
        }

        result.outcome = BattleOutcome::PLAYER_DEAD;
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "被 " << enemies[i].getName() << " 击败！\n";
        ConsoleUI::resetColor();

        const int fallbackHp = std::max(1, player.getMaxHp() / 4);
        BattleMapper::applyPlayerHp(player, fallbackHp);

        ConsoleUI::pause();
        return result;
    }

    result.outcome = BattleOutcome::PLAYER_WIN;
    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "击败了所有 " << defeated << " 个敌人！\n";
    ConsoleUI::resetColor();

    ConsoleUI::pause();
    return result;
}

BattleOutcome BattleSystem::fightOneEnemy(Character& player, Inventory& inventory, const Enemy& enemy) const
{
    return runCardBattle(player, inventory, enemy);
}

BattleOutcome BattleSystem::runCardBattle(Character& player, Inventory& inventory, const Enemy& enemy) const
{
    // Character / Enemy 先被投影为一次性的战斗快照；
    // 之后所有回合内状态都只修改快照，最后再把玩家 HP 写回角色对象。
    BattleActorState playerState = BattleMapper::fromCharacter(player);
    BattleActorState enemyState = BattleMapper::fromEnemy(enemy);
    BattleDeck playerDeck;
    BattleDeck enemyDeck;
    bool playerFirst = determinePlayerFirst(playerState, enemyState);
    int round = 1;

    while (playerState.isAlive() && enemyState.isAlive())
    {
        // 回合开始时只处理跨回合状态，不在这里抽牌或重置命中结果。
        beginRound(playerState, enemyState);

        BattleTurnContext context;
        context.round = round;
        context.playerFirst = playerFirst;
        context.playerHand = playerDeck.drawCards(kHandSize);
        context.enemyHand = enemyDeck.drawCards(kHandSize);

        // UI 展示和决策阶段完全基于 context + actorState，
        // 这样后续要替换成图形界面时也不会影响核心结算。
        ConsoleUI::clearScreen();
        printBattleState(playerState, enemyState, round, playerFirst);

        BattleDecision playerDecision;
        BattleDecision enemyDecision;

        if (playerFirst)
        {
            std::cout << "情报：你先手，观察到对方手牌：[ ";
            for (const auto& card : context.enemyHand)
                std::cout << card.getName() << " ";
            std::cout << "]\n";

            playerDecision = getPlayerDecision(context, playerState, enemyState, inventory);
            if (playerDecision.action == BattleAction::Escape)
            {
                BattleMapper::applyPlayerHp(player, playerState.currentHp);
                return BattleOutcome::PLAYER_ESCAPE;
            }

            if (playerDecision.action == BattleAction::None)
                continue;

            context.revealedPlayerAction = playerDecision.action;
            enemyDecision = BattleAI::decideEnemyAction(context, playerState, enemyState);
            context.revealedEnemyAction = enemyDecision.action;

            std::cout << enemyState.name << " 观察到你的意图，准备打出 ["
                      << toString(enemyDecision.action) << "]。\n";
        }
        else
        {
            enemyDecision = BattleAI::decideEnemyAction(context, playerState, enemyState);
            context.revealedEnemyAction = enemyDecision.action;
            std::cout << "情报：" << enemyState.name << " 先手，公开意图为 ["
                      << toString(enemyDecision.action) << "]。\n";

            playerDecision = getPlayerDecision(context, playerState, enemyState, inventory);
            if (playerDecision.action == BattleAction::Escape)
            {
                BattleMapper::applyPlayerHp(player, playerState.currentHp);
                return BattleOutcome::PLAYER_ESCAPE;
            }

            if (playerDecision.action == BattleAction::None)
                continue;

            context.revealedPlayerAction = playerDecision.action;
        }

        if (playerDecision.usedCommand)
        {
            playerState.commandUsed = true;
            if (playerDecision.action == BattleAction::UseItem)
                std::cout << "你发动了指令，强制使用战斗道具。\n";
            else
                std::cout << "你发动了指令，强制执行 [" << toString(playerDecision.action) << "]。\n";
        }
        if (enemyDecision.usedCommand)
        {
            enemyState.commandUsed = true;
            std::cout << enemyState.name << " 发动了指令，强制执行 ["
                      << toString(enemyDecision.action) << "]。\n";
        }

        // incomingDamageMultiplier 是“本回合局部状态”，每轮开始时都重置。
        std::cout << "\n[回合结算]\n";
        playerState.incomingDamageMultiplier = 1.0;
        enemyState.incomingDamageMultiplier = 1.0;

        applyDefenseResolution(
            playerState, playerDeck, playerDecision.action,
            enemyState, enemyDeck, enemyDecision.action);
        applyDefenseResolution(
            enemyState, enemyDeck, enemyDecision.action,
            playerState, playerDeck, playerDecision.action);

        // 行动执行顺序永远由先后手决定；防反/防空这类“宣言后即生效”的效果
        // 会在正式执行动作前先结算。
        if (playerFirst)
        {
            executeAction(playerState, enemyState, playerDecision, &inventory);
            if (enemyState.isAlive())
                executeAction(enemyState, playerState, enemyDecision, nullptr);
        }
        else
        {
            executeAction(enemyState, playerState, enemyDecision, nullptr);
            if (playerState.isAlive())
                executeAction(playerState, enemyState, playerDecision, &inventory);
        }

        std::cout << "\n回合结束 -> 你的 HP: " << playerState.currentHp
                  << " | 敌方 HP: " << enemyState.currentHp << "\n";

        BattleMapper::applyPlayerHp(player, playerState.currentHp);
        ConsoleUI::pause();

        playerFirst = !playerFirst;
        ++round;
    }

    BattleMapper::applyPlayerHp(player, playerState.currentHp);
    return playerState.isAlive() ? BattleOutcome::PLAYER_WIN : BattleOutcome::PLAYER_DEAD;
}

void BattleSystem::beginRound(BattleActorState& playerState, BattleActorState& enemyState) const
{
    auto activatePendingDisarm = [](BattleActorState& actor) {
        actor.disarmed = actor.pendingDisarm;
        actor.pendingDisarm = false;
    };

    activatePendingDisarm(playerState);
    activatePendingDisarm(enemyState);
}

void BattleSystem::printBattleState(
    const BattleActorState& playerState,
    const BattleActorState& enemyState,
    int round,
    bool playerFirst) const
{
    ConsoleUI::printTitle("卡牌战斗");
    std::cout << "第 " << round << " 回合  |  "
              << (playerFirst ? "你先手" : "敌方先手") << "\n\n";

    std::cout << playerState.name << " [" << toString(playerState.element) << "] "
              << "HP: " << playerState.currentHp << "/" << playerState.maxHp
              << " " << makeHpBar(playerState.currentHp, playerState.maxHp) << "\n";
    std::cout << "ATK: " << playerState.attack
              << "  DEF: " << playerState.defense
              << "  SPD: " << playerState.speed
              << "  易伤: " << playerState.vulnerabilityStacks
              << "  指令: " << (playerState.commandUsed ? "已用" : "可用") << "\n\n";

    std::cout << enemyState.name << " [" << toString(enemyState.element) << "] "
              << "HP: " << enemyState.currentHp << "/" << enemyState.maxHp
              << " " << makeHpBar(enemyState.currentHp, enemyState.maxHp) << "\n";
    std::cout << "ATK: " << enemyState.attack
              << "  DEF: " << enemyState.defense
              << "  SPD: " << enemyState.speed
              << "  易伤: " << enemyState.vulnerabilityStacks
              << "  指令: " << (enemyState.commandUsed ? "已用" : "可用") << "\n";
}

void BattleSystem::printHand(const std::vector<BattleCard>& hand, const std::string& ownerLabel) const
{
    std::cout << ownerLabel << "手牌：\n";
    for (size_t i = 0; i < hand.size(); ++i)
        std::cout << (i + 1) << ". " << hand[i].getName() << "\n";
}

BattleDecision BattleSystem::getPlayerDecision(
    const BattleTurnContext& context,
    const BattleActorState& playerState,
    const BattleActorState& enemyState,
    const Inventory& inventory) const
{
    while (true)
    {
        std::cout << "\n";
        printHand(context.playerHand, "你的");
        // 指令保留为二级菜单，避免未来接入战斗内背包时再次改动主选择流程。
        std::cout << "9. 使用指令（强制攻击/防御/回复/使用道具）\n";
        std::cout << "0. 逃跑\n";
        const int choice = ConsoleUI::readInt("请选择行动: ");

        if (choice == 0)
        {
            BattleDecision decision;
            decision.action = BattleAction::Escape;
            return decision;
        }

        if (choice == 9)
        {
            BattleDecision decision = chooseCommandAction(playerState, enemyState, inventory);
            if (decision.action != BattleAction::None)
                return decision;
            continue;
        }

        if (choice >= 1 && choice <= static_cast<int>(context.playerHand.size()))
        {
            BattleDecision decision;
            decision.selectedIndex = choice - 1;
            decision.action = static_cast<BattleAction>(context.playerHand[choice - 1].getActionId());
            return decision;
        }

        std::cout << "无效选择，请重新输入。\n";
    }
}

BattleDecision BattleSystem::chooseCommandAction(
    const BattleActorState& playerState,
    const BattleActorState& enemyState,
    const Inventory& inventory) const
{
    (void)enemyState;

    // 指令是“每场战斗一次”的资源，而不是每回合一次。
    if (playerState.commandUsed)
    {
        std::cout << "你的指令已经用过了。\n";
        return {};
    }

    while (true)
    {
        std::cout << "\n[指令菜单]\n";
        std::cout << "1. 强制攻击\n";
        std::cout << "2. 强制防御\n";
        std::cout << "3. 强制回复\n";
        std::cout << "4. 战斗中使用背包物品\n";
        std::cout << "0. 返回\n";
        const int choice = ConsoleUI::readInt("请选择指令: ");

        if (choice == 0)
            return {};

        if (choice >= 1 && choice <= 3)
        {
            BattleDecision decision;
            decision.usedCommand = true;
            decision.action = static_cast<BattleAction>(choice);
            return decision;
        }

        if (choice == 4)
        {
            const std::vector<int> usableIndices = collectBattleUsableItems(inventory);
            if (usableIndices.empty())
            {
                std::cout << "当前背包里没有可在战斗中使用的回复类道具。\n";
                continue;
            }

            std::cout << "\n[可用战斗道具]\n";
            for (size_t i = 0; i < usableIndices.size(); ++i)
            {
                const Item* item = inventory.getItem(usableIndices[i]);
                if (!item) continue;
                std::cout << (i + 1) << ". [" << item->getType() << "] "
                          << item->getName() << " | " << item->getEffectDescription() << "\n";
            }
            std::cout << "0. 返回\n";

            const int itemChoice = ConsoleUI::readInt("请选择道具: ");
            if (itemChoice == 0)
                continue;

            if (itemChoice >= 1 && itemChoice <= static_cast<int>(usableIndices.size()))
            {
                BattleDecision decision;
                decision.usedCommand = true;
                decision.usedBattleItem = true;
                decision.action = BattleAction::UseItem;
                decision.inventoryIndex = usableIndices[itemChoice - 1];
                return decision;
            }

            std::cout << "无效选择，请重新输入。\n";
            continue;
        }

        std::cout << "无效选择，请重新输入。\n";
    }
}

void BattleSystem::applyDefenseResolution(
    BattleActorState& actor,
    BattleDeck& actorDeck,
    BattleAction actorAction,
    BattleActorState& target,
    BattleDeck& targetDeck,
    BattleAction targetAction) const
{
    if (actorAction != BattleAction::Defense)
        return;

    if (targetAction == BattleAction::Attack)
    {
        // 防反只让对手“下回合”缴械，不直接抹掉本回合已宣言的攻击。
        std::cout << "【防反触发】" << actor.name << " 识破了 " << target.name
                  << " 的攻击，令其下回合缴械，并永久失去一张攻击牌！\n";
        target.pendingDisarm = true;
        targetDeck.permanentlyRemoveAttackCard();
        actor.incomingDamageMultiplier = 0.35;
        return;
    }

    std::cout << "【防空惩罚】" << actor.name
              << " 的防御落空，挂上 1 层易伤，并永久失去一张防御牌！\n";
    actor.vulnerabilityStacks++;
    actorDeck.permanentlyRemoveDefenseCard();
}

void BattleSystem::executeAction(
    BattleActorState& actor,
    BattleActorState& target,
    const BattleDecision& decision,
    Inventory* inventory) const
{
    if (!actor.isAlive())
        return;

    // 这里不处理 Escape；逃跑在决策阶段就已经直接返回到上层流程。
    if (decision.action == BattleAction::Attack)
    {
        if (actor.disarmed)
        {
            std::cout << actor.name << " 因缴械状态，本回合无法攻击。\n";
            actor.disarmed = false;
            return;
        }

        // 伤害公式保留现项目的 attack / defense 主轴，
        // 再叠加元素克制、防御减伤和易伤层数。
        const int damage = calculateDamage(actor, target);
        target.currentHp = std::max(0, target.currentHp - damage);
        std::cout << actor.name << " 打出攻击牌，对 " << target.name
                  << " 造成了 " << damage << " 点伤害。\n";
        return;
    }

    if (decision.action == BattleAction::Defense)
    {
        actor.incomingDamageMultiplier = std::min(actor.incomingDamageMultiplier, 0.35);
        std::cout << actor.name << " 进入防御姿态。\n";
        return;
    }

    if (decision.action == BattleAction::Heal)
    {
        // 回复量与攻击属性挂钩，延续参考项目里“进攻型角色回复更强”的倾向。
        const int healAmount = calculateHealAmount(actor);
        const int before = actor.currentHp;
        actor.currentHp = std::min(actor.maxHp, actor.currentHp + healAmount);
        std::cout << actor.name << " 打出回复牌，恢复了 "
                  << (actor.currentHp - before) << " 点生命。\n";
    }

    if (decision.action == BattleAction::UseItem)
    {
        if (inventory == nullptr || decision.inventoryIndex < 0)
            return;

        const Item* item = inventory->getItem(decision.inventoryIndex);
        if (item == nullptr || !item->canUseInBattle())
        {
            std::cout << actor.name << " 试图使用战斗道具，但该道具当前不可用。\n";
            return;
        }

        const int healAmount = std::max(0, item->getBattleHealAmount());
        const int before = actor.currentHp;
        actor.currentHp = std::min(actor.maxHp, actor.currentHp + healAmount);
        std::cout << actor.name << " 使用了 [" << item->getName() << "]，恢复了 "
                  << (actor.currentHp - before) << " 点生命。\n";
        inventory->removeItem(decision.inventoryIndex);
    }
}

int BattleSystem::calculateDamage(const BattleActorState& attacker, const BattleActorState& defender) const
{
    // 先按现项目属性体系得到基础伤害，再叠加卡牌战斗的状态修正。
    int damage = std::max(1, attacker.attack - defender.defense);
    if (hasTypeAdvantage(attacker.element, defender.element))
        damage *= 2;

    double finalDamage = static_cast<double>(damage);
    finalDamage *= defender.incomingDamageMultiplier;
    finalDamage *= (1.0 + 0.5 * defender.vulnerabilityStacks);
    return std::max(1, static_cast<int>(finalDamage));
}

int BattleSystem::calculateHealAmount(const BattleActorState& actor) const
{
    return std::max(1, actor.attack * 8 / 10);
}

std::vector<int> BattleSystem::collectBattleUsableItems(const Inventory& inventory) const
{
    std::vector<int> indices;
    for (int i = 0; i < inventory.size(); ++i)
    {
        const Item* item = inventory.getItem(i);
        if (item != nullptr && item->canUseInBattle())
            indices.push_back(i);
    }
    return indices;
}

bool BattleSystem::determinePlayerFirst(const BattleActorState& playerState, const BattleActorState& enemyState) const
{
    // 先比较速度；平局时再回落到一个简单稳定的综合战力判定。
    if (playerState.speed != enemyState.speed)
        return playerState.speed > enemyState.speed;

    const int playerPower = playerState.attack + playerState.defense + playerState.currentHp / 10;
    const int enemyPower = enemyState.attack + enemyState.defense + enemyState.currentHp / 10;
    return playerPower >= enemyPower;
}

std::string BattleSystem::makeHpBar(int hp, int maxHp, int width) const
{
    if (maxHp <= 0) return "[无效]";
    const int safeHp = std::max(0, std::min(hp, maxHp));
    const int filled = safeHp * width / maxHp;
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}
