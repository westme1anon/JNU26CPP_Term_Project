#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif

void ConsoleUI::setupConsole()
{
#ifdef _WIN32
    // 设置控制台标题
    SetConsoleTitleA("Campus RPG Adventure");

    // 设置控制台输出编码为 UTF-8，便于显示中文
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 设置窗口大小与背景颜色
    // color 0B 表示黑色背景，亮青色文字
    system("mode con cols=100 lines=35");
    system("color 0B");
#endif
}

void ConsoleUI::clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleUI::pause()
{
    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void ConsoleUI::setColor(int color)
{
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
    (void)color;
#endif
}

void ConsoleUI::resetColor()
{
    setColor(GameConfig::COLOR_DEFAULT);
}

void ConsoleUI::printTitle(const std::string& title)
{
    setColor(GameConfig::COLOR_TITLE);
    printLine('=');
    std::cout << "                 " << title << "\n";
    printLine('=');
    resetColor();
}

void ConsoleUI::printBox(const std::vector<std::string>& lines)
{
    const int width = 60;

    std::cout << "+" << std::string(width, '-') << "+\n";

    for (const auto& line : lines)
    {
        std::cout << "| " << line;
        if (static_cast<int>(line.size()) < width - 1)
        {
            std::cout << std::string(width - 1 - line.size(), ' ');
        }
        std::cout << "|\n";
    }

    std::cout << "+" << std::string(width, '-') << "+\n";
}

void ConsoleUI::printLine(char ch, int length)
{
    std::cout << std::string(length, ch) << "\n";
}

int ConsoleUI::readInt(const std::string& prompt)
{
    int value;

    while (true)
    {
        std::cout << prompt;
        std::cin >> value;

        if (!std::cin.fail())
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        setColor(GameConfig::COLOR_ERROR);
        std::cout << "输入无效，请输入数字。\n";
        resetColor();
    }
}
