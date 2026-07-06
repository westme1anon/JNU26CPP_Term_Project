// Item.cpp
#include "Item.h"
#include "Character.h"

#include <iostream>

Item::Item(const std::string& name, const std::string& description, int price)
    : name(name), description(description), price(price)
{
}

std::string Item::getName() const
{
    return name;
}

std::string Item::getDescription() const
{
    return description;
}

int Item::getPrice() const
{
    return price;
}

FoodItem::FoodItem(const std::string& name, const std::string& description, int price, int recoverHp)
    : Item(name, description, price), recoverHp(recoverHp)
{
}

void FoodItem::use(Character& player)
{
    // TODO: 调用 player.heal(recoverHp)，恢复生命值。
}

std::string FoodItem::getType() const
{
    return "食物";
}

std::unique_ptr<Item> FoodItem::clone() const
{
    return std::make_unique<FoodItem>(*this);
}

MedicineItem::MedicineItem(const std::string& name, const std::string& description, int price, int healAmount)
    : Item(name, description, price), healAmount(healAmount)
{
}

void MedicineItem::use(Character& player)
{
    // TODO: 调用 player.heal(healAmount)，恢复更多生命值。
}

std::string MedicineItem::getType() const
{
    return "药品";
}

std::unique_ptr<Item> MedicineItem::clone() const
{
    return std::make_unique<MedicineItem>(*this);
}

EquipmentItem::EquipmentItem(
    const std::string& name,
    const std::string& description,
    int price,
    int attackBonus,
    int defenseBonus
)
    : Item(name, description, price),
      attackBonus(attackBonus),
      defenseBonus(defenseBonus)
{
}

void EquipmentItem::use(Character& player)
{
    // TODO: 增加玩家攻击力和防御力。
}

std::string EquipmentItem::getType() const
{
    return "装备";
}

std::unique_ptr<Item> EquipmentItem::clone() const
{
    return std::make_unique<EquipmentItem>(*this);
}