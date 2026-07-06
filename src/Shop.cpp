// Shop.cpp
#include "Shop.h"
#include "ConsoleUI.h"

#include <iostream>

Shop::Shop()
{
    loadGoods();
}

void Shop::loadGoods()
{
    // TODO: 从 data/shop_items.txt 加载商品。
    // 临时可加入食物、药品、装备三类物品。
}

void Shop::showGoods() const
{
    // TODO: 遍历 goods，显示商品编号、名称、类型、价格、描述。
}

bool Shop::buyItem(int index, Character& player, Inventory& inventory)
{
    // TODO:
    // 1. 校验 index
    // 2. 判断金币是否足够
    // 3. 扣除金币
    // 4. 克隆商品加入背包
    return false;
}

bool Shop::sellItem(int index, Character& player, Inventory& inventory)
{
    // TODO:
    // 1. 从背包取出物品
    // 2. 按价格一定比例折算金币
    // 3. 给玩家增加金币
    return false;
}