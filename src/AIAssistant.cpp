// AIAssistant.cpp
#include "AIAssistant.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#if defined(ENABLE_CURL_AI)
#include <curl/curl.h>
#endif

#if defined(ENABLE_CURL_AI)
// 静态回调函数，将 libcurl 接收到的响应数据写入 string 对象
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);
    if (str)
        str->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
#endif

AIAssistant::AIAssistant() {
    initTips();
}

void AIAssistant::showTips(const Character& player) const {
    if (player.getHp() < player.getMaxHp() / 3) {
        // 生命值较低提示使用药品或补充生命
        auto it = tips.find("low_hp");
        if (it != tips.end()) {
            std::cout << it->second << std::endl;
        }
    }
    if (player.getGold() > 200) {
        // 金币较多提示购买装备
        auto it = tips.find("high_gold");
        if (it != tips.end()) {
            std::cout << it->second << std::endl;
        }
    }
    if (player.getLevel() < 2) {
        // 等级较低提示提升等级
        auto it = tips.find("low_level");
        if (it != tips.end()) {
            std::cout << it->second << std::endl;
        }
    }
}

std::string AIAssistant::suggestAction(const Character& player, const Inventory& inventory) const {
    // 构造玩家当前状态的描述字符串
    std::ostringstream stateStream;
    stateStream << "玩家" << player.getName()
                << "，等级" << player.getLevel()
                << "，生命值" << player.getHp() << "/" << player.getMaxHp()
                << "，金币" << player.getGold() << "。";
    if (inventory.isEmpty()) {
        stateStream << " 背包当前为空。";
    } else {
        stateStream << " 背包共有" << inventory.size() << "件物品。";
    }
    std::string gameState = stateStream.str();

    // 检查 DeepSeek API Key 是否存在
#if defined(ENABLE_CURL_AI)
    const char* apiKey = std::getenv("DEEPSEEK_API_KEY");
    if (apiKey && *apiKey != '\0') {
        // DeepSeek API 接口配置
        std::string url = "https://api.deepseek.com/chat/completions";
        std::string modelName = "deepseek-v4-pro";

        // 构造 DeepSeek API 的系统提示 (System message) 和用户消息 (User message)
        std::string systemContent =
            "你是校园RPG游戏 CampusRPG 的AI助手。游戏包含角色成长（玩家通过任务和战斗获取经验提升等级）、"
            "背包与物品管理（背包可存储食物、药品、装备，道具用于回复生命或提升属性）、校园商店（购买或出售物品）、"
            "任务系统（接受任务并完成获得奖励）、战斗系统（挑战校园内的敌人获得经验和金币）。"
            "你的任务是根据游戏规则和玩家当前状态，为玩家提供下一步行动的建议。";
        std::string userContent =
            "当前玩家状态：" + gameState +
            " 请根据上述游戏规则和玩家状态，提供一个简短的下一步行动建议。";

        // 构造 POST 请求的 JSON 请求体
        std::ostringstream json;
        json << "{"
             << "\"model\":\"" << modelName << "\","
             << "\"messages\":["
             << "{\"role\":\"system\",\"content\":\"" << systemContent << "\"},"
             << "{\"role\":\"user\",\"content\":\"" << userContent << "\"}"
             << "],"
             << "\"thinking\":{\"type\":\"enabled\"},"
             << "\"reasoning_effort\":\"high\","
             << "\"stream\":false"
             << "}";

        std::string requestBody = json.str();

        // 初始化 libcurl 并设置请求参数
        curl_global_init(CURL_GLOBAL_DEFAULT);
        CURL* curl = curl_easy_init();
        if (curl) {
            // 设置 HTTP 请求的 URL、POST 方法和请求体
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());

            // 设置 HTTP 头部，包括内容类型和认证信息
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            std::string authHeader = std::string("Authorization: Bearer ") + apiKey;
            headers = curl_slist_append(headers, authHeader.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // 设置回调函数以获取响应数据
            std::string responseData;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

            // 执行 HTTP POST 请求
            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                // 请求成功，尝试从 JSON 响应中提取 AI 给出的建议
                std::string suggestion;
                size_t contentPos = responseData.find("\"content\"");
                if (contentPos != std::string::npos) {
                    // 查找 content 键对应的值的起止位置（假设 DeepSeek 返回的 JSON 兼容 OpenAI API 格式）
                    size_t start = responseData.find('\"', contentPos + 9);
                    if (start != std::string::npos) {
                        size_t end = responseData.find('\"', start + 1);
                        if (end != std::string::npos && end > start) {
                            suggestion = responseData.substr(start + 1, end - start - 1);
                        }
                    }
                }
                if (!suggestion.empty()) {
                    // 清理并返回AI建议
                    curl_easy_cleanup(curl);
                    curl_slist_free_all(headers);
                    curl_global_cleanup();
                    return suggestion;
                }
            } else {
                std::cerr << "[AI助手] DeepSeek API调用失败，错误码: " << res << std::endl;
            }
            // 清理CURL资源
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            curl_global_cleanup();
        }
    }
#endif

    // 如果没有可用的AI服务或API调用失败，则使用规则系统提供默认建议
    if (player.getHp() < player.getMaxHp() / 3) {
        return tips.at("low_hp");
    }
    if (inventory.isEmpty()) {
        return tips.at("empty_inventory");
    }
    if (player.getGold() > 200) {
        return tips.at("high_gold");
    }
    if (player.getLevel() < 2) {
        return tips.at("low_level");
    }
    // 默认通用提示
    return "暂无特别建议，继续探索校园提升自己吧。";
}

void AIAssistant::initTips() {
    tips.clear();
    // 初始化规则提示
    tips["low_hp"] = "【提示】你的生命值较低，建议使用药品回复或前往商店购买药品。";
    tips["high_gold"] = "【提示】你持有大量金币，可以去校园商店购买更强力的装备。";
    tips["low_level"] = "【提示】当前等级较低，可以尝试完成任务或挑战低等级敌人来升级。";
    tips["empty_inventory"] = "【提示】你的背包为空，去校园商店购买一些道具吧。";
}