#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>

enum class ThemeType {
    Dark,
    Light,
    Cyberpunk
};

struct Theme {
    sf::Color bg;
    sf::Color text;
    sf::Color border;

    // Visual elements color coding
    sf::Color primary;    // Blue - current element/processing
    sf::Color secondary;  // Yellow - pivot/special elements
    sf::Color success;    // Green - sorted/completed path
    sf::Color danger;     // Red - compared/obstacle
    sf::Color normal;     // Neutral - active list/neutral node
    sf::Color inactive;   // Base background/unvisited elements
    sf::Color auxiliary;  // Magenta/Cyan for secondary pointers
};

class ThemeManager {
public:
    static ThemeManager& getInstance();

    void setTheme(ThemeType type);
    ThemeType getCurrentThemeType() const { return m_currentType; }
    const Theme& getCurrentTheme() const { return m_theme; }

    // Synchronize ImGui style configuration with the active theme colors
    void applyImGuiStyle();

    // Utility to convert sf::Color to ImVec4 for ImGui
    static ImVec4 toImVec4(const sf::Color& color) {
        return ImVec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
    }

private:
    ThemeManager();
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    ThemeType m_currentType;
    Theme m_theme;

    void loadDarkTheme();
    void loadLightTheme();
    void loadCyberpunkTheme();
};
