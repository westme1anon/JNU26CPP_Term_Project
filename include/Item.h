// Item.h
#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <memory>

class Character;

// ============================================================
// Item
// ------------------------------------------------------------
// 物品抽象基类。
// 通过多态支持食物、药品、装备等不同类型物品。
// ============================================================

class Item
{
protected:
    std::string name;
    std::string description;
    int price;

public:
    Item(const std::string& name, const std::string& description, int price);
    virtual ~Item() = default;

    // 使用物品。
    // 不同子类实现不同效果。
    virtual void use(Character& player) = 0;

    // 获取物品类型名称。
    virtual std::string getType() const = 0;

    // 获取效果描述（例如 "HP+15" 或 "ATK+8 DEF+2"）。
    virtual std::string getEffectDescription() const = 0;

    // 克隆物品对象。
    // 便于商店商品复制到背包中。
    virtual std::unique_ptr<Item> clone() const = 0;

    std::string getName() const;
    std::string getDescription() const;
    int getPrice() const;
};

// 食物类物品：一般用于少量恢复生命值。
class FoodItem : public Item
{
private:
    int recoverHp;

public:
    friend class SaveManager; // 声明 SaveManager 为友元类，以便访问私有成员 recoverHp
    FoodItem(const std::string& name, const std::string& description, int price, int recoverHp);

    void use(Character& player) override;
    std::string getType() const override;
    std::string getEffectDescription() const override;
    std::unique_ptr<Item> clone() const override;
};

// 药品类物品：一般用于较大幅度治疗或特殊恢复。
class MedicineItem : public Item
{
private:
    int healAmount;

public:
    friend class SaveManager; // 声明 SaveManager 为友元类，以便访问私有成员 healAmount
    MedicineItem(const std::string& name, const std::string& description, int price, int healAmount);

    void use(Character& player) override;
    std::string getType() const override;
    std::string getEffectDescription() const override;
    std::unique_ptr<Item> clone() const override;
};

// 装备类物品：用于提升角色攻击或防御属性。
class EquipmentItem : public Item
{
private:
    int attackBonus;
    int defenseBonus;

public:
    friend class SaveManager; // 声明 SaveManager 为友元类，以便访问私有成员 attackBonus 和 defenseBonus
    EquipmentItem(
        const std::string& name,
        const std::string& description,
        int price,
        int attackBonus,
        int defenseBonus
    );

    void use(Character& player) override;
    std::string getType() const override;
    std::string getEffectDescription() const override;
    std::unique_ptr<Item> clone() const override;
};

#endif