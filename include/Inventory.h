// Inventory.h
#ifndef INVENTORY_H
#define INVENTORY_H

#include "Item.h"

#include <vector>
#include <memory>

// ============================================================
// Inventory
// ------------------------------------------------------------
// 背包类。
// 使用 vector<unique_ptr<Item>> 管理物品。
// 支持获得、查看、使用、删除、出售物品。
// ============================================================

class Inventory
{
private:
    std::vector<std::unique_ptr<Item>> items;

public:
    Inventory() = default;

    // 添加物品到背包。
    void addItem(std::unique_ptr<Item> item);

    // 显示背包内全部物品。
    void showItems() const;

    // 使用指定下标的物品。
    // 使用成功返回 true。
    bool useItem(int index, Character& player);

    // 删除指定下标的物品。
    bool removeItem(int index);

    // 将指定物品从背包取出，用于出售。
    std::unique_ptr<Item> sellItem(int index);

    // 判断背包是否为空。
    bool isEmpty() const;

    // 获取物品数量。
    int size() const;

    // 获取指定下标物品的只读指针。
    const Item* getItem(int index) const;
};

#endif