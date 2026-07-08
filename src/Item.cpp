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

// 使用食物：恢复生命值。
void FoodItem::use(Character& player)
{
    std::cout << "使用了 " << getName() << "，恢复了 " << recoverHp << " 点生命值！\n";
    player.heal(recoverHp);
}

std::string FoodItem::getType() const
{
    return "食物";
}

std::string FoodItem::getEffectDescription() const
{
    return "HP+" + std::to_string(recoverHp);
}

std::unique_ptr<Item> FoodItem::clone() const
{
    return std::make_unique<FoodItem>(*this);
}

MedicineItem::MedicineItem(const std::string& name, const std::string& description, int price, int healAmount)
    : Item(name, description, price), healAmount(healAmount)
{
}

// 使用药品：恢复大量生命值。
void MedicineItem::use(Character& player)
{
    std::cout << "使用了 " << getName() << "，恢复了 " << healAmount << " 点生命值！\n";
    player.heal(healAmount);
}

std::string MedicineItem::getType() const
{
    return "药品";
}

std::string MedicineItem::getEffectDescription() const
{
    return "HP+" + std::to_string(healAmount);
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

// 使用装备：增加攻击力和防御力。
void EquipmentItem::use(Character& player)
{
    std::cout << "装备了 " << getName()
              << "，攻击力 +" << attackBonus
              << "，防御力 +" << defenseBonus << "！\n";
    player.addAttack(attackBonus);
    player.addDefense(defenseBonus);
}

std::string EquipmentItem::getType() const
{
    return "装备";
}

std::string EquipmentItem::getEffectDescription() const
{
    return "ATK+" + std::to_string(attackBonus) + " DEF+" + std::to_string(defenseBonus);
}

std::unique_ptr<Item> EquipmentItem::clone() const
{
    return std::make_unique<EquipmentItem>(*this);
}
