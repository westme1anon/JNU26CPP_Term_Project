// ============================================================
// main.cpp — SFML 3 图形化重构
// ------------------------------------------------------------
// 将原控制台 RPG 重构为 SFML 图形化原型。
// 使用几何图形和颜色块替代所有美术素材。
//
// 架构要点：
//   - 数据与渲染分离：逻辑类保持纯净，渲染通过 draw() 完成
//   - 标准游戏主循环：Event → Update → Render
//   - 无字体依赖：全部使用方框和颜色表达含义
// ============================================================

#include <SFML/Graphics.hpp>

#include "include/Character.h"
#include "include/GameConfig.h"

#include <iostream>

// ============================================================
// Button — 简易交互按钮
// ------------------------------------------------------------
// 矩形按钮，支持悬停变色和点击回调。
// 纯图形占位符，用于代替文本菜单。
// ============================================================
class Button
{
public:
    Button(float x, float y, float w, float h,
           sf::Color normal, sf::Color hover)
        : m_normalColor(normal)
        , m_hoverColor(hover)
    {
        m_shape.setPosition({x, y});
        m_shape.setSize({w, h});
        m_shape.setFillColor(normal);
    }

    // 每帧调用：根据鼠标位置更新悬停状态
    void update(const sf::RenderWindow& window)
    {
        const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        const sf::FloatRect bounds = m_shape.getGlobalBounds();
        m_hovered = bounds.contains(static_cast<sf::Vector2f>(mousePos));
        m_shape.setFillColor(m_hovered ? m_hoverColor : m_normalColor);
    }

    bool isHovered() const { return m_hovered; }

    void draw(sf::RenderWindow& window) const
    {
        window.draw(m_shape);
    }

private:
    sf::RectangleShape m_shape;
    sf::Color          m_normalColor;
    sf::Color          m_hoverColor;
    bool               m_hovered = false;
};

// ============================================================
// main
// ============================================================
int main()
{
    // ---- 创建窗口 ----
    sf::RenderWindow window(
        sf::VideoMode({GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT}),
        GameConfig::WINDOW_TITLE,
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(GameConfig::FRAME_RATE_LIMIT);

    // ---- 创建玩家 ----
    Character player;
    player.create("Hero");

    // ---- 创建互动按钮（右侧） ----
    // 正常：蓝色；悬停：深蓝色
    Button interactBtn(
        1050.f, 300.f, 180.f, 60.f,
        sf::Color(0, 100, 200),   // normal blue
        sf::Color(0, 60, 140)     // hover dark blue
    );

    // 深灰色背景
    const sf::Color bgColor(30, 30, 30);

    // ============================================================
    // 游戏主循环：Event → Update → Render
    // ============================================================
    while (window.isOpen())
    {
        // ========================================================
        // Phase 1: 处理输入事件 (Event)
        // ========================================================
        while (const std::optional sfEvent = window.pollEvent())
        {
            // 窗口关闭
            if (sfEvent->is<sf::Event::Closed>())
            {
                window.close();
            }
            // ESC 键退出
            else if (const auto* key = sfEvent->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
            }
            // 鼠标点击：检测按钮
            else if (sfEvent->is<sf::Event::MouseButtonPressed>())
            {
                if (interactBtn.isHovered())
                {
                    std::cout << "[Debug]: \347\202\271\345\207\273\344\272\206\344\272\222\345\212\250\346\214\211\351\222\256" << std::endl;
                }
            }
        }

        // ========================================================
        // Phase 2: 更新游戏逻辑 (Update)
        // ========================================================
        interactBtn.update(window);
        // 未来可在此处扩展：player 移动、敌人 AI、碰撞检测等

        // ========================================================
        // Phase 3: 渲染绘制 (Render)
        // ========================================================
        window.clear(bgColor);

        player.draw(window);       // 玩家（绿色方块 + 血条）
        interactBtn.draw(window);  // 互动按钮

        window.display();
    }

    return 0;
}
