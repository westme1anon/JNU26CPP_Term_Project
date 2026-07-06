#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <string>
#include <vector>

// ============================================================
// ConsoleUI
// ------------------------------------------------------------
// 控制台界面辅助类。
// 负责清屏、设置颜色、打印标题、打印边框菜单等。
// 目标是让 CMD 界面更接近图形化体验。
// ============================================================

class ConsoleUI
{
public:
    // 初始化控制台环境。
    // 例如设置窗口标题、窗口大小、编码、背景色等。
    static void setupConsole();

    // 清空控制台屏幕。
    static void clearScreen();

    // 暂停程序，等待用户按键继续。
    static void pause();

    // 设置控制台文本颜色。
    static void setColor(int color);

    // 恢复默认控制台颜色。
    static void resetColor();

    // 打印居中风格标题。
    static void printTitle(const std::string& title);

    // 使用 ASCII 边框打印多行内容，用于模拟图形化菜单。
    static void printBox(const std::vector<std::string>& lines);

    // 打印分隔线。
    static void printLine(char ch = '=', int length = 60);

    // 安全读取整数输入，避免非法输入导致程序崩溃。
    static int readInt(const std::string& prompt);
};

#endif