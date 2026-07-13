// PathUtil.h
#ifndef PATH_UTIL_H
#define PATH_UTIL_H

#include <string>
#include <filesystem>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace PathUtil
{

inline bool isDebugLoggingEnabled()
{
#ifdef _WIN32
    char* env = nullptr;
    std::size_t length = 0;
    const errno_t result = _dupenv_s(&env, &length, "PATHUTIL_DEBUG");
    const bool enabled = (result == 0 && env != nullptr && env[0] != '\0' && env[0] != '0');
    if (env != nullptr)
    {
        free(env);
    }
    return enabled;
#else
    const char* env = std::getenv("PATHUTIL_DEBUG");
    return env != nullptr && env[0] != '\0' && env[0] != '0';
#endif
}

inline void logDebug(const std::string& message)
{
    if (isDebugLoggingEnabled())
    {
        std::cout << message << "\n";
    }
}

inline std::string getExecutableDir()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
    {
        std::filesystem::path exePath(buffer);
        return exePath.parent_path().string();
    }
#endif
    return std::filesystem::current_path().string();
}

inline std::string searchUpward(const std::string& startDir, const std::string& relativePath)
{
    std::filesystem::path current = std::filesystem::absolute(startDir);
    std::filesystem::path target(relativePath);

    for (int i = 0; i < 10; ++i)
    {
        std::filesystem::path candidate = current / target;
        if (std::filesystem::exists(candidate))
            return candidate.string();

        std::filesystem::path parent = current.parent_path();
        if (parent == current) break;
        current = parent;
    }
    return "";
}

inline std::string resolvePath(const std::string& relativePath)
{
    if (std::filesystem::path(relativePath).is_absolute())
    {
        if (std::filesystem::exists(relativePath))
            return relativePath;
        logDebug("[PathUtil] 绝对路径不存在: " + relativePath);
        return "";
    }

    std::string cwd = std::filesystem::current_path().string();
    std::string exeDir = getExecutableDir();

    logDebug("[PathUtil] 查找 \"" + relativePath + "\"");
    logDebug("[PathUtil]   CWD=" + cwd);

    // 1. 从 CWD 向上搜索
    std::string resolved = searchUpward(cwd, relativePath);
    if (!resolved.empty())
    {
        logDebug("[PathUtil]   => 找到(CWD向上): " + resolved);
        return resolved;
    }

    // 2. 从 exe 目录向上搜索
    if (exeDir != cwd)
    {
        logDebug("[PathUtil]   EXE=" + exeDir);
        resolved = searchUpward(exeDir, relativePath);
        if (!resolved.empty())
        {
            logDebug("[PathUtil]   => 找到(EXE向上): " + resolved);
            return resolved;
        }
    }

    // 3. CWD直接
    std::filesystem::path directPath = std::filesystem::path(cwd) / relativePath;
    if (std::filesystem::exists(directPath))
    {
        logDebug("[PathUtil]   => 找到(CWD直接): " + directPath.string());
        return directPath.string();
    }

    // 4. EXE直接
    std::filesystem::path exeDirectPath = std::filesystem::path(exeDir) / relativePath;
    if (std::filesystem::exists(exeDirectPath))
    {
        logDebug("[PathUtil]   => 找到(EXE直接): " + exeDirectPath.string());
        return exeDirectPath.string();
    }

    logDebug("[PathUtil]   => 未找到!");
    return "";
}

} // namespace PathUtil

#endif
