// Shop.cpp
#include "Shop.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>
#include <fstream>
#include <sstream>

Shop::Shop()
{
    loadGoods();
}

void Shop::loadGoods()
{
    goods.clear();

    std::ifstream file(GameConfig::SHOP_ITEMS_FILE_PATH);
    if (!file.is_open())
    {
        std::cerr << "警告: 无法打开商店数据文件 " << GameConfig::SHOP_ITEMS_FILE_PATH << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::stringstream ss(line);
        std::string type, name, description, priceStr, value1Str, value2Str;

        std::getline(ss, type, '|');
        std::getline(ss, name, '|');
        std::getline(ss, description, '|');
        std::getline(ss, priceStr, '|');
        std::getline(ss, value1Str, '|');
        std::getline(ss, value2Str, '|');

        int price = std::stoi(priceStr);
        int value1 = std::stoi(value1Str);
        int value2 = std::stoi(value2Str);

        if (type == "Food")
        {
            goods.push_back(std::make_unique<FoodItem>(name, description, price, value1));
        }
        else if (type == "Medicine")
        {
            goods.push_back(std::make_unique<MedicineItem>(name, description, price, value1));
        }
        else if (type == "Equipment")
        {
            goods.push_back(std::make_unique<EquipmentItem>(name, description, price, value1, value2));
        }
    }
}

void Shop::showGoods() const
{
    if (goods.empty())
    {
        std::cout << "商店暂无商品。\n";
        return;
    }

    ConsoleUI::printLine('-', 70);

    // 表头
    std::cout << "编号  名称                类型       价格      描述\n";
    ConsoleUI::printLine('-', 70);

    for (size_t i = 0; i < goods.size(); ++i)
    {
        std::cout << (i + 1) << "     ";
        std::cout.width(20);
        std::cout << std::left << goods[i]->getName();
        std::cout.width(10);
        std::cout << std::left << goods[i]->getType();
        std::cout << goods[i]->getPrice() << " 金币  ";
        std::cout << goods[i]->getDescription() << "\n";
    }

    ConsoleUI::printLine('-', 70);
}

bool Shop::buyItem(int index, Character& player, Inventory& inventory)
{
    if (index < 0 || index >= static_cast<int>(goods.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的商品编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    int price = goods[index]->getPrice();

    if (!player.costGold(price))
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "金币不足！需要 " << price << " 金币，当前拥有 " << player.getGold() << " 金币。\n";
        ConsoleUI::resetColor();
        return false;
    }

    inventory.addItem(goods[index]->clone());

    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "成功购买 " << goods[index]->getName() << "，花费 " << price << " 金币。\n";
    ConsoleUI::resetColor();

    return true;
}

bool Shop::sellItem(int index, Character& player, Inventory& inventory)
{
    if (inventory.isEmpty())
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "背包为空，没有可出售的物品。\n";
        ConsoleUI::resetColor();
        return false;
    }

    if (index < 0 || index >= inventory.size())
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的物品编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    const Item* item = inventory.getItem(index);
    if (item == nullptr)
    {
        return false;
    }

    int sellPrice = item->getPrice() / 2;

    std::unique_ptr<Item> removed = inventory.sellItem(index);
    if (removed == nullptr)
    {
        return false;
    }

    player.gainGold(sellPrice);

    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "成功出售 " << removed->getName() << "，获得 " << sellPrice << " 金币。\n";
    ConsoleUI::resetColor();

    return true;
}
