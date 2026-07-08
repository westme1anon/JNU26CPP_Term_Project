// Inventory.cpp
#include "Inventory.h"
#include "ConsoleUI.h"

#include <iostream>

// 添加物品到背包。
void Inventory::addItem(std::unique_ptr<Item> item)
{
    items.push_back(std::move(item));
}

// 显示背包内全部物品。
void Inventory::showItems() const
{
    if (items.empty())
    {
        std::cout << "背包为空。\n";
        return;
    }

    std::cout << "── 背包物品 ──\n";
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        std::cout << (i + 1) << ". [" << items[i]->getType() << "] "
                  << items[i]->getName()
                  << " | 售价: " << items[i]->getPrice()
                  << " | " << items[i]->getDescription() << "\n";
    }
}

// 使用指定下标的物品（0-based 索引）。使用后从背包删除。
bool Inventory::useItem(int index, Character& player)
{
    if (index < 0 || index >= static_cast<int>(items.size()))
    {
        std::cout << "编号无效！\n";
        return false;
    }

    items[index]->use(player);
    items.erase(items.begin() + index);
    return true;
}

// 删除指定下标的物品（0-based 索引）。
bool Inventory::removeItem(int index)
{
    if (index < 0 || index >= static_cast<int>(items.size()))
    {
        std::cout << "编号无效！\n";
        return false;
    }

    items.erase(items.begin() + index);
    std::cout << "物品已删除。\n";
    return true;
}

// 将指定物品从背包取出（0-based 索引），用于出售。
std::unique_ptr<Item> Inventory::sellItem(int index)
{
    if (index < 0 || index >= static_cast<int>(items.size()))
    {
        std::cout << "编号无效！\n";
        return nullptr;
    }

    std::unique_ptr<Item> item = std::move(items[index]);
    items.erase(items.begin() + index);
    return item;
}

bool Inventory::isEmpty() const
{
    return items.empty();
}

int Inventory::size() const
{
    return static_cast<int>(items.size());
}

// 获取指定下标物品的只读指针（0-based 索引）。
const Item* Inventory::getItem(int index) const
{
    if (index < 0 || index >= static_cast<int>(items.size()))
    {
        return nullptr;
    }

    return items[index].get();
}
