#include "ThemeManager.hpp"

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() {
    setTheme(ThemeType::Dark);
}

void ThemeManager::setTheme(ThemeType type) {
    m_currentType = type;
    switch (type) {
        case ThemeType::Dark:
            loadDarkTheme();
            break;
        case ThemeType::Light:
            loadLightTheme();
            break;
        case ThemeType::Cyberpunk:
            loadCyberpunkTheme();
            break;
    }
    applyImGuiStyle();
}

void ThemeManager::loadDarkTheme() {
    m_theme.bg = sf::Color(18, 18, 24);
    m_theme.text = sf::Color(220, 220, 224);
    m_theme.border = sf::Color(58, 58, 64);
    m_theme.primary = sf::Color(52, 152, 219);     // Blue
    m_theme.secondary = sf::Color(241, 196, 15);   // Yellow
    m_theme.success = sf::Color(46, 204, 113);     // Green
    m_theme.danger = sf::Color(231, 76, 60);       // Red
    m_theme.normal = sf::Color(100, 110, 120);     // Muted Blue-grey
    m_theme.inactive = sf::Color(35, 35, 45);      // Dark slate
    m_theme.auxiliary = sf::Color(155, 89, 182);   // Purple
}

void ThemeManager::loadLightTheme() {
    m_theme.bg = sf::Color(245, 246, 250);
    m_theme.text = sf::Color(47, 53, 66);
    m_theme.border = sf::Color(200, 205, 215);
    m_theme.primary = sf::Color(24, 116, 205);
    m_theme.secondary = sf::Color(218, 165, 32);
    m_theme.success = sf::Color(34, 139, 34);
    m_theme.danger = sf::Color(220, 20, 60);
    m_theme.normal = sf::Color(150, 160, 170);
    m_theme.inactive = sf::Color(225, 228, 235);
    m_theme.auxiliary = sf::Color(139, 0, 139);
}

void ThemeManager::loadCyberpunkTheme() {
    m_theme.bg = sf::Color(10, 5, 20);
    m_theme.text = sf::Color(0, 255, 220);         // Neon Cyan
    m_theme.border = sf::Color(255, 0, 128);       // Neon Pink-Red
    m_theme.primary = sf::Color(0, 255, 255);      // Cyan
    m_theme.secondary = sf::Color(255, 240, 0);    // Neon Yellow
    m_theme.success = sf::Color(50, 255, 50);      // Neon Green
    m_theme.danger = sf::Color(255, 0, 85);        // Pinkish Red
    m_theme.normal = sf::Color(110, 0, 220);       // Violet
    m_theme.inactive = sf::Color(25, 10, 50);      // Deep Indigo
    m_theme.auxiliary = sf::Color(255, 0, 255);    // Magenta
}

void ThemeManager::applyImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounding & Layout details for premium feel
    style.WindowRounding = 6.f;
    style.ChildRounding = 4.f;
    style.FrameRounding = 4.f;
    style.GrabRounding = 4.f;
    style.PopupRounding = 4.f;
    style.ScrollbarRounding = 4.f;
    style.TabRounding = 4.f;
    style.WindowBorderSize = 1.f;
    style.FrameBorderSize = 1.f;

    ImVec4* colors = style.Colors;
    
    // Core color conversions
    ImVec4 bg = toImVec4(m_theme.bg);
    ImVec4 text = toImVec4(m_theme.text);
    ImVec4 border = toImVec4(m_theme.border);
    ImVec4 primary = toImVec4(m_theme.primary);
    ImVec4 inactive = toImVec4(m_theme.inactive);

    // Apply color palettes
    colors[ImGuiCol_Text]                   = text;
    colors[ImGuiCol_TextDisabled]           = ImVec4(text.x, text.y, text.z, 0.5f);
    colors[ImGuiCol_WindowBg]               = bg;
    colors[ImGuiCol_ChildBg]                = ImVec4(bg.x * 0.9f, bg.y * 0.9f, bg.z * 0.9f, 1.0f);
    colors[ImGuiCol_PopupBg]                = bg;
    colors[ImGuiCol_Border]                 = border;
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg]                = inactive;
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(inactive.x * 1.3f, inactive.y * 1.3f, inactive.z * 1.3f, 1.0f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(inactive.x * 1.6f, inactive.y * 1.6f, inactive.z * 1.6f, 1.0f);
    colors[ImGuiCol_TitleBg]                = bg;
    colors[ImGuiCol_TitleBgActive]          = bg;
    colors[ImGuiCol_TitleBgCollapsed]       = bg;
    colors[ImGuiCol_MenuBarBg]              = bg;
    colors[ImGuiCol_ScrollbarBg]            = bg;
    colors[ImGuiCol_ScrollbarGrab]          = border;
    colors[ImGuiCol_ScrollbarGrabHovered]   = primary;
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(primary.x * 0.8f, primary.y * 0.8f, primary.z * 0.8f, 1.0f);
    colors[ImGuiCol_CheckMark]              = primary;
    colors[ImGuiCol_SliderGrab]             = primary;
    colors[ImGuiCol_SliderGrabActive]        = ImVec4(primary.x * 0.8f, primary.y * 0.8f, primary.z * 0.8f, 1.0f);
    colors[ImGuiCol_Button]                 = ImVec4(primary.x, primary.y, primary.z, 0.4f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(primary.x, primary.y, primary.z, 0.7f);
    colors[ImGuiCol_ButtonActive]           = primary;
    colors[ImGuiCol_Header]                 = ImVec4(primary.x, primary.y, primary.z, 0.3f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(primary.x, primary.y, primary.z, 0.5f);
    colors[ImGuiCol_HeaderActive]           = primary;
    colors[ImGuiCol_Separator]              = border;
    colors[ImGuiCol_SeparatorHovered]       = primary;
    colors[ImGuiCol_SeparatorActive]        = primary;
    colors[ImGuiCol_ResizeGrip]             = border;
    colors[ImGuiCol_ResizeGripHovered]      = primary;
    colors[ImGuiCol_ResizeGripActive]       = primary;
    colors[ImGuiCol_Tab]                    = ImVec4(primary.x, primary.y, primary.z, 0.2f);
    colors[ImGuiCol_TabHovered]             = ImVec4(primary.x, primary.y, primary.z, 0.5f);
    colors[ImGuiCol_TabActive]              = ImVec4(primary.x, primary.y, primary.z, 0.6f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(primary.x, primary.y, primary.z, 0.1f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(primary.x, primary.y, primary.z, 0.3f);
    colors[ImGuiCol_PlotLines]              = primary;
    colors[ImGuiCol_PlotLinesHovered]       = toImVec4(m_theme.danger);
    colors[ImGuiCol_PlotHistogram]          = primary;
    colors[ImGuiCol_PlotHistogramHovered]   = toImVec4(m_theme.danger);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(primary.x, primary.y, primary.z, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = toImVec4(m_theme.secondary);
}
