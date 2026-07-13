// PathUtil.h
#ifndef PATH_UTIL_H
#define PATH_UTIL_H

#include <string>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace PathUtil
{

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
        std::cout << "[PathUtil] 绝对路径不存在: " << relativePath << "\n";
        return "";
    }

    std::string cwd = std::filesystem::current_path().string();
    std::string exeDir = getExecutableDir();

    std::cout << "[PathUtil] 查找 \"" << relativePath << "\"\n";
    std::cout << "[PathUtil]   CWD=" << cwd << "\n";

    // 1. 从 CWD 向上搜索
    std::string resolved = searchUpward(cwd, relativePath);
    if (!resolved.empty())
    {
        std::cout << "[PathUtil]   => 找到(CWD向上): " << resolved << "\n";
        return resolved;
    }

    // 2. 从 exe 目录向上搜索
    if (exeDir != cwd)
    {
        std::cout << "[PathUtil]   EXE=" << exeDir << "\n";
        resolved = searchUpward(exeDir, relativePath);
        if (!resolved.empty())
        {
            std::cout << "[PathUtil]   => 找到(EXE向上): " << resolved << "\n";
            return resolved;
        }
    }

    // 3. CWD直接
    std::filesystem::path directPath = std::filesystem::path(cwd) / relativePath;
    if (std::filesystem::exists(directPath))
    {
        std::cout << "[PathUtil]   => 找到(CWD直接): " << directPath.string() << "\n";
        return directPath.string();
    }

    // 4. EXE直接
    std::filesystem::path exeDirectPath = std::filesystem::path(exeDir) / relativePath;
    if (std::filesystem::exists(exeDirectPath))
    {
        std::cout << "[PathUtil]   => 找到(EXE直接): " << exeDirectPath.string() << "\n";
        return exeDirectPath.string();
    }

    std::cout << "[PathUtil]   => 未找到!\n";
    return "";
}

} // namespace PathUtil

#endif
