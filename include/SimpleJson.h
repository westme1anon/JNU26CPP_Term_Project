// SimpleJson.h
#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <string>
#include <vector>
#include <stdexcept>
#include <map>

// ============================================================
// JsonValue
// ------------------------------------------------------------
// 简易 JSON 值类型，支持对象、数组、字符串、整数。
// 用于游戏配置文件解析，不追求完整 JSON 规范。
// ============================================================

class JsonValue
{
public:
    enum Type { Null, Object, Array, String, Integer };

    Type type = Null;
    std::string stringValue;
    int intValue = 0;
    std::vector<JsonValue> arrayItems;
    std::vector<std::pair<std::string, JsonValue>> objectMembers;

    JsonValue() : type(Null) {}

    // ---- 访问器 ----

    bool has(const std::string& key) const
    {
        if (type != Object) return false;
        for (const auto& m : objectMembers)
            if (m.first == key) return true;
        return false;
    }

    const JsonValue& operator[](const std::string& key) const
    {
        if (type != Object)
            throw std::runtime_error("JsonValue::operator[] key on non-object");
        for (const auto& m : objectMembers)
            if (m.first == key) return m.second;
        throw std::runtime_error("JsonValue::operator[] key not found: " + key);
    }

    const JsonValue& operator[](size_t index) const
    {
        if (type != Array)
            throw std::runtime_error("JsonValue::operator[] index on non-array");
        if (index >= arrayItems.size())
            throw std::runtime_error("JsonValue::operator[] index out of bounds");
        return arrayItems[index];
    }

    size_t size() const
    {
        if (type == Array)  return arrayItems.size();
        if (type == Object) return objectMembers.size();
        return 0;
    }

    std::string asString() const
    {
        if (type == String) return stringValue;
        if (type == Integer) return std::to_string(intValue);
        return "";
    }

    int asInt() const
    {
        if (type == Integer) return intValue;
        if (type == String) return std::stoi(stringValue);
        return 0;
    }
};

// ============================================================
// 公开解析 API
// ============================================================

JsonValue parseJson(const std::string& source);
JsonValue parseJsonFile(const std::string& filepath);

#endif
