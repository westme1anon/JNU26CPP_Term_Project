// SimpleJson.cpp
#include "SimpleJson.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>

// ============================================================
// 内部：简易递归下降 JSON 解析器
// ============================================================

namespace {

class Parser
{
public:
    explicit Parser(const std::string& source)
        : src(source), pos(0) {}

    JsonValue parseValue()
    {
        skipWhitespace();
        if (pos >= src.size())
            throw std::runtime_error("Unexpected end of JSON input");

        char c = src[pos];
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
            return parseInteger();
        if (c == 'n') { expectLiteral("null");  return JsonValue(); }
        if (c == 't') { expectLiteral("true");  JsonValue v; v.type = JsonValue::Integer; v.intValue = 1; return v; }
        if (c == 'f') { expectLiteral("false"); JsonValue v; v.type = JsonValue::Integer; v.intValue = 0; return v; }

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

private:
    const std::string& src;
    size_t pos;

    void skipWhitespace()
    {
        while (pos < src.size() &&
               (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r'))
            ++pos;
    }

    void expectChar(char expected)
    {
        skipWhitespace();
        if (pos >= src.size() || src[pos] != expected)
            throw std::runtime_error(std::string("Expected '") + expected + "'");
        ++pos;
    }

    void expectLiteral(const std::string& literal)
    {
        for (char ch : literal)
        {
            if (pos >= src.size() || src[pos] != ch)
                throw std::runtime_error("Expected literal: " + literal);
            ++pos;
        }
    }

    JsonValue parseObject()
    {
        JsonValue obj;
        obj.type = JsonValue::Object;
        expectChar('{');
        skipWhitespace();
        if (pos < src.size() && src[pos] == '}')
        {
            ++pos;
            return obj;
        }
        while (true)
        {
            skipWhitespace();
            std::string key = parseRawString();
            expectChar(':');
            JsonValue val = parseValue();
            obj.objectMembers.emplace_back(key, std::move(val));
            skipWhitespace();
            if (pos < src.size() && src[pos] == ',')
            {
                ++pos;
                continue;
            }
            break;
        }
        expectChar('}');
        return obj;
    }

    JsonValue parseArray()
    {
        JsonValue arr;
        arr.type = JsonValue::Array;
        expectChar('[');
        skipWhitespace();
        if (pos < src.size() && src[pos] == ']')
        {
            ++pos;
            return arr;
        }
        while (true)
        {
            arr.arrayItems.push_back(parseValue());
            skipWhitespace();
            if (pos < src.size() && src[pos] == ',')
            {
                ++pos;
                continue;
            }
            break;
        }
        expectChar(']');
        return arr;
    }

    std::string parseRawString()
    {
        std::string result;
        expectChar('"');
        while (pos < src.size() && src[pos] != '"')
        {
            if (src[pos] == '\\')
            {
                ++pos;
                if (pos >= src.size()) break;
                switch (src[pos])
                {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case '/':  result += '/'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += src[pos]; break;
                }
            }
            else
            {
                result += src[pos];
            }
            ++pos;
        }
        expectChar('"');
        return result;
    }

    JsonValue parseString()
    {
        JsonValue val;
        val.type = JsonValue::String;
        val.stringValue = parseRawString();
        return val;
    }

    JsonValue parseInteger()
    {
        size_t start = pos;
        if (pos < src.size() && src[pos] == '-') ++pos;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos])))
            ++pos;
        JsonValue val;
        val.type = JsonValue::Integer;
        val.intValue = std::stoi(src.substr(start, pos - start));
        return val;
    }
};

} // anonymous namespace

// ============================================================
// 公开 API
// ============================================================

JsonValue parseJson(const std::string& source)
{
    Parser parser(source);
    JsonValue root = parser.parseValue();
    return root;
}

JsonValue parseJsonFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open JSON file: " + filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseJson(buffer.str());
}
