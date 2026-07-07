#include "EnvLoader.h"

#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
std::string trim(const std::string& value)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos)
    {
        return "";
    }

    const std::size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

std::string stripQuotes(const std::string& value)
{
    if (value.size() >= 2)
    {
        const char first = value.front();
        const char last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
        {
            return value.substr(1, value.size() - 2);
        }
    }

    return value;
}

bool hasEnvironmentValue(const std::string& key)
{
    const char* value = std::getenv(key.c_str());
    return value != nullptr && value[0] != '\0';
}

void setEnvironmentValue(const std::string& key, const std::string& value)
{
#if defined(_WIN32)
    _putenv_s(key.c_str(), value.c_str());
#else
    setenv(key.c_str(), value.c_str(), 0);
#endif
}
}

bool loadEnvFile(const std::string& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(input, line))
    {
        std::string content = trim(line);
        if (content.empty() || content[0] == '#')
        {
            continue;
        }

        if (content.rfind("export ", 0) == 0)
        {
            content = trim(content.substr(7));
        }

        const std::size_t delimiter = content.find('=');
        if (delimiter == std::string::npos)
        {
            continue;
        }

        const std::string key = trim(content.substr(0, delimiter));
        if (key.empty() || hasEnvironmentValue(key))
        {
            continue;
        }

        std::string value = trim(content.substr(delimiter + 1));
        value = stripQuotes(value);
        setEnvironmentValue(key, value);
    }

    return true;
}
