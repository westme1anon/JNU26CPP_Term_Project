#include "AIAssistant.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
enum class AiFallbackReason
{
    None,
    MissingApiKey,
    MissingScript,
    MissingPython,
    RequestFailed,
    TempFileFailed
};

struct AiQueryResult
{
    std::string suggestion;
    AiFallbackReason fallbackReason = AiFallbackReason::None;
};

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

std::filesystem::path executableDirectory()
{
#if defined(_WIN32)
    std::vector<char> buffer(MAX_PATH);
    DWORD length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (length == buffer.size())
    {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    if (length == 0)
    {
        return {};
    }
    return std::filesystem::path(std::string(buffer.data(), length)).parent_path();
#else
    return {};
#endif
}

std::filesystem::path findFileUpwards(
    const std::filesystem::path& startDir,
    const std::filesystem::path& relativePath,
    int maxLevels = 6
)
{
    if (startDir.empty())
    {
        return {};
    }

    std::filesystem::path current = startDir;
    for (int level = 0; level <= maxLevels; ++level)
    {
        const std::filesystem::path candidate = current / relativePath;
        if (std::filesystem::exists(candidate))
        {
            return candidate;
        }

        if (!current.has_parent_path())
        {
            break;
        }

        const std::filesystem::path parent = current.parent_path();
        if (parent == current)
        {
            break;
        }
        current = parent;
    }

    return {};
}

std::filesystem::path findPythonScriptPath()
{
    const std::filesystem::path relativeScript = std::filesystem::path("scripts") / "ai_helper.py";

    if (std::filesystem::exists(relativeScript))
    {
        return relativeScript;
    }

    const std::filesystem::path cwdMatch =
        findFileUpwards(std::filesystem::current_path(), relativeScript);
    if (!cwdMatch.empty())
    {
        return cwdMatch;
    }

    return findFileUpwards(executableDirectory(), relativeScript);
}

std::string buildGameState(const Character& player, const Inventory& inventory)
{
    std::ostringstream stream;
    stream << "玩家: " << player.getName() << "\n";
    stream << "等级: " << player.getLevel() << "\n";
    stream << "生命值: " << player.getHp() << "/" << player.getMaxHp() << "\n";
    stream << "金币: " << player.getGold() << "\n";
    stream << "背包物品数: " << inventory.size() << "\n";
    stream << "背包是否为空: " << (inventory.isEmpty() ? "是" : "否") << "\n";
    return stream.str();
}

std::string runCommandAndCapture(const std::string& command)
{
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe)
    {
        return "";
    }

    std::array<char, 256> buffer{};
    std::string output;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
    {
        output += buffer.data();
    }

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

std::string buildLocalSuggestion(
    const Character& player,
    const Inventory& inventory,
    const std::unordered_map<std::string, std::string>& tips
)
{
    if (player.getHp() < player.getMaxHp() / 3)
    {
        return tips.at("low_hp");
    }
    if (inventory.isEmpty())
    {
        return tips.at("empty_inventory");
    }
    if (player.getGold() > 200)
    {
        return tips.at("high_gold");
    }
    if (player.getLevel() < 2)
    {
        return tips.at("low_level");
    }
    return "【本地建议】当前状态稳定，继续探索校园、完成任务或挑战敌人即可。";
}

std::string buildFallbackPrefix(AiFallbackReason reason)
{
    switch (reason)
    {
    case AiFallbackReason::MissingApiKey:
        return "【本地模式】未检测到 DEEPSEEK_API_KEY，已切换为本地建议：";
    case AiFallbackReason::MissingScript:
        return "【本地模式】未找到 AI 脚本，已切换为本地建议：";
    case AiFallbackReason::MissingPython:
        return "【本地模式】未检测到 Python 环境，已切换为本地建议：";
    case AiFallbackReason::RequestFailed:
        return "【本地模式】联网 AI 请求失败，已切换为本地建议：";
    case AiFallbackReason::TempFileFailed:
        return "【本地模式】无法准备 AI 请求数据，已切换为本地建议：";
    case AiFallbackReason::None:
    default:
        return "";
    }
}

AiQueryResult queryPythonAssistant(const std::string& gameState)
{
#if defined(_WIN32)
    char* apiKeyBuffer = nullptr;
    std::size_t apiKeyLength = 0;
    if (_dupenv_s(&apiKeyBuffer, &apiKeyLength, "DEEPSEEK_API_KEY") != 0 ||
        apiKeyBuffer == nullptr || apiKeyLength <= 1)
    {
        free(apiKeyBuffer);
        return {"", AiFallbackReason::MissingApiKey};
    }
#else
    const char* apiKeyBuffer = std::getenv("DEEPSEEK_API_KEY");
    if (!apiKeyBuffer || *apiKeyBuffer == '\0')
    {
        return {"", AiFallbackReason::MissingApiKey};
    }
#endif

    const std::filesystem::path scriptPath = findPythonScriptPath();
    if (scriptPath.empty())
    {
#if defined(_WIN32)
        free(apiKeyBuffer);
#endif
        return {"", AiFallbackReason::MissingScript};
    }

    const std::filesystem::path inputPath =
        std::filesystem::temp_directory_path() / "campusrpg_ai_request.txt";
    {
        std::ofstream output(inputPath, std::ios::binary);
        if (!output.is_open())
        {
#if defined(_WIN32)
            free(apiKeyBuffer);
#endif
            return {"", AiFallbackReason::TempFileFailed};
        }
        output << gameState;
    }

    const std::string quotedScript = "\"" + scriptPath.string() + "\"";
    const std::string quotedInput = "\"" + inputPath.string() + "\"";

    const std::string pyLauncherOutput = trim(
        runCommandAndCapture("py -3 -X utf8 " + quotedScript + " " + quotedInput + " 2>NUL"));
    if (!pyLauncherOutput.empty())
    {
#if defined(_WIN32)
        free(apiKeyBuffer);
#endif
        return {pyLauncherOutput, AiFallbackReason::None};
    }

    const std::string pythonOutput = trim(
        runCommandAndCapture("python -X utf8 " + quotedScript + " " + quotedInput + " 2>NUL"));
#if defined(_WIN32)
    free(apiKeyBuffer);
#endif
    if (!pythonOutput.empty())
    {
        return {pythonOutput, AiFallbackReason::None};
    }

    const std::string pyCheck = trim(runCommandAndCapture("py -3 --version 2>NUL"));
    const std::string pythonCheck = trim(runCommandAndCapture("python --version 2>NUL"));
    if (pyCheck.empty() && pythonCheck.empty())
    {
        return {"", AiFallbackReason::MissingPython};
    }

    return {"", AiFallbackReason::RequestFailed};
}
}

AIAssistant::AIAssistant()
{
    initTips();
}

void AIAssistant::showTips(const Character& player) const
{
    if (player.getHp() < player.getMaxHp() / 3)
    {
        auto it = tips.find("low_hp");
        if (it != tips.end())
        {
            std::cout << it->second << std::endl;
        }
    }

    if (player.getGold() > 200)
    {
        auto it = tips.find("high_gold");
        if (it != tips.end())
        {
            std::cout << it->second << std::endl;
        }
    }

    if (player.getLevel() < 2)
    {
        auto it = tips.find("low_level");
        if (it != tips.end())
        {
            std::cout << it->second << std::endl;
        }
    }
}

std::string AIAssistant::suggestAction(const Character& player, const Inventory& inventory) const
{
    const AiQueryResult result = queryPythonAssistant(buildGameState(player, inventory));
    if (!result.suggestion.empty())
    {
        return result.suggestion;
    }

    const std::string localSuggestion = buildLocalSuggestion(player, inventory, tips);
    const std::string prefix = buildFallbackPrefix(result.fallbackReason);
    if (!prefix.empty())
    {
        return prefix + localSuggestion;
    }
    return localSuggestion;
}

void AIAssistant::initTips()
{
    tips.clear();
    tips["low_hp"] = "【本地建议】你的生命值偏低，建议先使用恢复类道具，或去商店补充药品。";
    tips["high_gold"] = "【本地建议】你当前金币较多，可以优先去商店更新更强的装备。";
    tips["low_level"] = "【本地建议】你当前等级偏低，建议先完成任务或挑战较弱的敌人。";
    tips["empty_inventory"] = "【本地建议】你的背包是空的，建议先去商店购买基础道具。";
}
