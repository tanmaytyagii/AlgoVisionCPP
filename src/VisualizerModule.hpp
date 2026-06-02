#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <cstdarg>

namespace ImGui {
    inline void Metric(const char* label, const char* format, ...) {
        ImGui::Text("%s:", label);
        ImGui::SameLine();
        
        va_list args;
        va_start(args, format);
        // Highlight metric value in nice sky blue
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.f, 1.f));
        ImGui::TextV(format, args);
        ImGui::PopStyleColor();
        va_end(args);
    }
}

/**
 * @brief Base class for all visualizer modules (Sorting, Searching, Graph, etc.)
 */
class VisualizerModule {
public:
    virtual ~VisualizerModule() = default;

    /**
     * @brief Handle input events (clicks, keypresses) specific to this module.
     * @param event The SFML event to process.
     */
    virtual void handleEvent(const sf::Event& event) = 0;

    /**
     * @brief Update the internal state of the visualizer (animations, timers).
     * @param dt Time elapsed since the last frame (in seconds).
     */
    virtual void update(float dt) = 0;

    /**
     * @brief Render the visual components (bars, nodes, edges) using SFML.
     * @param window The target SFML RenderWindow.
     */
    virtual void render(sf::RenderWindow& window) = 0;

    /**
     * @brief Render the sidebar controls, information, and statistics using Dear ImGui.
     */
    virtual void renderUI() = 0;

    /**
     * @brief Reset the state of the visualizer module.
     */
    virtual void reset() = 0;
};
