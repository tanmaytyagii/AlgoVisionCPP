#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>
#include "VisualizerModule.hpp"
#include "algorithms/SortingAlgorithms.hpp"

enum class ActiveTab {
    Sorting,
    Searching,
    Graph,
    Pathfinding,
    DataStructures,
    PerformanceCompare,
    Settings
};

class App {
public:
    App();
    ~App();

    /**
     * @brief Starts the application main loop.
     */
    void run();

private:
    void initWindow();
    void initImGui();
    void processEvents();
    void update(float dt);
    void render();
    void renderUI();

    // UI Panel Builders
    void drawSidebar();
    void drawTopbar();
    void drawBottomPanel();
    void drawMainWorkspace();

    // Main App components
    sf::RenderWindow m_window;
    bool m_running;

    // Active visualizer state
    ActiveTab m_currentTab;
    std::unique_ptr<VisualizerModule> m_sortingVisualizer;
    std::unique_ptr<VisualizerModule> m_searchingVisualizer;
    std::unique_ptr<VisualizerModule> m_graphVisualizer;
    std::unique_ptr<VisualizerModule> m_pathfindingVisualizer;
    std::unique_ptr<VisualizerModule> m_dsVisualizer;

    // Performance comparison variables
    bool m_sideBySideMode;
    std::unique_ptr<SortingAlgorithms> m_compareVisualizer1;
    std::unique_ptr<SortingAlgorithms> m_compareVisualizer2;
    int m_compareAlgo1;
    int m_compareAlgo2;
    int m_compareSize;
    
    // FPS and execution timing
    float m_fps;
    float m_frameTime;
};
