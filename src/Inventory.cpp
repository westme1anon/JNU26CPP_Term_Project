// Inventory.cpp
#include "Inventory.h"
#include "ConsoleUI.h"

#include <iostream>

void Inventory::addItem(std::unique_ptr<Item> item)
{
    // TODO: 将物品加入 items。
}

void Inventory::showItems() const
{
    // TODO: 遍历 items，显示物品编号、名称、类型、价格、描述。
}

bool Inventory::useItem(int index, Character& player)
{
    // TODO: 校验 index，调用物品 use 方法，使用后根据规则决定是否删除。
    return false;
}

bool Inventory::removeItem(int index)
{
    // TODO: 校验 index，从 vector 中删除对应物品。
    return false;
}

std::unique_ptr<Item> Inventory::sellItem(int index)
{
    // TODO: 校验 index，将对应 unique_ptr 移出并从背包删除。
    return nullptr;
}

bool Inventory::isEmpty() const
{
    return items.empty();
}

int Inventory::size() const
{
    return static_cast<int>(items.size());
}

const Item* Inventory::getItem(int index) const
{
    // TODO: 校验 index 并返回只读指针。
    return nullptr;
}