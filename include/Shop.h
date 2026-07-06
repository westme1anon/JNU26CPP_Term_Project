// Shop.h
#ifndef SHOP_H
#define SHOP_H

#include "Item.h"
#include "Inventory.h"
#include "Character.h"

#include <vector>
#include <memory>

// ============================================================
// Shop
// ------------------------------------------------------------
// 商店系统。
// 管理商品列表，支持查看商品、购买物品、出售背包物品。
// ============================================================

class Shop
{
private:
    std::vector<std::unique_ptr<Item>> goods;

public:
    Shop();

    // 加载商品数据。
    void loadGoods();

    // 显示商店商品列表。
    void showGoods() const;

    // 玩家购买商品。
    bool buyItem(int index, Character& player, Inventory& inventory);

    // 玩家出售背包中的物品。
    bool sellItem(int index, Character& player, Inventory& inventory);
};

#endif