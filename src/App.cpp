#include "App.hpp"
#include "ui/ThemeManager.hpp"
#include "algorithms/SortingAlgorithms.hpp"
#include "algorithms/SearchingAlgorithms.hpp"
#include "graph/GraphVisualizer.hpp"
#include "pathfinding/PathfindingVisualizer.hpp"
#include "data_structures/DSVisualizer.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

App::App() 
    : m_running(false)
    , m_currentTab(ActiveTab::Sorting)
    , m_sideBySideMode(false)
    , m_compareAlgo1(4) // Quick Sort
    , m_compareAlgo2(3) // Merge Sort
    , m_compareSize(80)
    , m_fps(0.f)
    , m_frameTime(0.f) {
    initWindow();
    initImGui();

    // Instantiate visualizers
    m_sortingVisualizer = std::make_unique<SortingAlgorithms>();
    m_searchingVisualizer = std::make_unique<SearchingAlgorithms>();
    m_graphVisualizer = std::make_unique<GraphVisualizer>();
    m_pathfindingVisualizer = std::make_unique<PathfindingVisualizer>();
    m_dsVisualizer = std::make_unique<DSVisualizer>();

    m_compareVisualizer1 = std::make_unique<SortingAlgorithms>();
    m_compareVisualizer2 = std::make_unique<SortingAlgorithms>();
}

App::~App() {
    ImGui::SFML::Shutdown();
}

void App::initWindow() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8; // Smooth graphics edges

    m_window.create(sf::VideoMode({1440, 900}), "AlgoVisionCPP - Visualize, Learn, Master", sf::Style::Default, sf::State::Windowed, settings);
    m_window.setFramerateLimit(60);
}

void App::initImGui() {
    if (!ImGui::SFML::Init(m_window)) {
        std::cerr << "Failed to initialize ImGui-SFML!\n";
    }

    // Set styling
    ThemeManager::getInstance().applyImGuiStyle();
}

void App::run() {
    m_running = true;
    sf::Clock deltaClock;

    while (m_running && m_window.isOpen()) {
        processEvents();

        float dt = deltaClock.restart().asSeconds();
        m_fps = 1.f / (dt > 0.f ? dt : 0.016f);
        m_frameTime = dt * 1000.f; // in ms

        update(dt);
        render();
    }
}

void App::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            m_running = false;
        }
        else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            // Adjust viewport
            sf::FloatRect visibleArea(sf::Vector2f(0.f, 0.f), sf::Vector2f(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)));
            m_window.setView(sf::View(visibleArea));
        }

        // Delegate input to active visualizer module if ImGui is not capturing mouse/keyboard
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            switch (m_currentTab) {
                case ActiveTab::Sorting:
                    m_sortingVisualizer->handleEvent(*event);
                    break;
                case ActiveTab::Searching:
                    m_searchingVisualizer->handleEvent(*event);
                    break;
                case ActiveTab::Graph:
                    m_graphVisualizer->handleEvent(*event);
                    break;
                case ActiveTab::Pathfinding:
                    m_pathfindingVisualizer->handleEvent(*event);
                    break;
                case ActiveTab::DataStructures:
                    m_dsVisualizer->handleEvent(*event);
                    break;
                default:
                    break;
            }
        }
    }
}

void App::update(float dt) {
    ImGui::SFML::Update(m_window, sf::seconds(dt));

    if (m_sideBySideMode) {
        m_compareVisualizer1->update(dt);
        m_compareVisualizer2->update(dt);
        return;
    }

    switch (m_currentTab) {
        case ActiveTab::Sorting:
            m_sortingVisualizer->update(dt);
            break;
        case ActiveTab::Searching:
            m_searchingVisualizer->update(dt);
            break;
        case ActiveTab::Graph:
            m_graphVisualizer->update(dt);
            break;
        case ActiveTab::Pathfinding:
            m_pathfindingVisualizer->update(dt);
            break;
        case ActiveTab::DataStructures:
            m_dsVisualizer->update(dt);
            break;
        default:
            break;
    }
}

void App::renderUI() {
    // Top-level dockspace / custom window layout for sidebars
    drawTopbar();
    drawSidebar();
    drawBottomPanel();
    drawMainWorkspace();
}

void App::drawTopbar() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_window.getSize().x), 55));
    ImGui::Begin("Topbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().primary), "▲ AlgoVisionCPP");
    ImGui::SameLine();
    ImGui::TextDisabled(" |  Visualize. Learn. Master Algorithms.");

    // Theme selector
    ImGui::SameLine(ImGui::GetWindowWidth() - 360);
    ImGui::SetNextItemWidth(140);
    const char* themes[] = { "Dark Theme", "Light Theme", "Cyberpunk" };
    int currentThemeIndex = static_cast<int>(ThemeManager::getInstance().getCurrentThemeType());
    if (ImGui::Combo("Theme", &currentThemeIndex, themes, IM_ARRAYSIZE(themes))) {
        ThemeManager::getInstance().setTheme(static_cast<ThemeType>(currentThemeIndex));
    }

    // Performance info
    ImGui::SameLine(ImGui::GetWindowWidth() - 150);
    ImGui::TextDisabled("FPS: %.1f (%.1f ms)", m_fps, m_frameTime);

    ImGui::End();
}

void App::drawSidebar() {
    ImGui::SetNextWindowPos(ImVec2(0, 55));
    ImGui::SetNextWindowSize(ImVec2(220, static_cast<float>(m_window.getSize().y) - 55));
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::Spacing();
    ImGui::Text("MODULES");
    ImGui::Separator();
    ImGui::Spacing();

    auto drawNavButton = [this](const char* label, ActiveTab tab) {
        bool selected = (m_currentTab == tab);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().primary));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        }

        if (ImGui::Button(label, ImVec2(200, 40))) {
            m_currentTab = tab;
            m_sideBySideMode = (tab == ActiveTab::PerformanceCompare);
        }

        ImGui::PopStyleColor();
        ImGui::Spacing();
    };

    drawNavButton("░  Sorting Visualizer", ActiveTab::Sorting);
    drawNavButton("⚲  Searching Visualizer", ActiveTab::Searching);
    drawNavButton("◈  Graph Visualizer", ActiveTab::Graph);
    drawNavButton("巴  Pathfinding Visualizer", ActiveTab::Pathfinding);
    drawNavButton("◇  Data Structures", ActiveTab::DataStructures);
    drawNavButton("⚖  Compare Sorts", ActiveTab::PerformanceCompare);

    ImGui::End();
}

void App::drawBottomPanel() {
    // Define position and sizes
    float sidebarWidth = 220.f;
    float bottomPanelHeight = 220.f;
    
    ImGui::SetNextWindowPos(ImVec2(sidebarWidth, static_cast<float>(m_window.getSize().y) - bottomPanelHeight));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_window.getSize().x) - sidebarWidth, bottomPanelHeight));
    ImGui::Begin("Status & Learning Panel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    if (m_sideBySideMode) {
        const char* sorts[] = { "Bubble Sort", "Selection Sort", "Insertion Sort", "Merge Sort", "Quick Sort", "Heap Sort", "Counting Sort", "Radix Sort" };
        
        ImGui::Columns(3, "CompareMetricsTable", true);
        ImGui::SetColumnWidth(0, 320);
        ImGui::SetColumnWidth(1, 320);
        
        // Left Algo Details
        ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().primary), "%s (Left Panel)", sorts[m_compareAlgo1]);
        ImGui::Separator();
        ImGui::Metric("Timer", "%.2fs", m_compareVisualizer1->getElapsedTime());
        ImGui::Text("Comparisons: %d", m_compareVisualizer1->getComparisons());
        ImGui::Text("Swaps: %d", m_compareVisualizer1->getSwaps());
        
        ImGui::NextColumn();
        
        // Right Algo Details
        ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().primary), "%s (Right Panel)", sorts[m_compareAlgo2]);
        ImGui::Separator();
        ImGui::Metric("Timer", "%.2fs", m_compareVisualizer2->getElapsedTime());
        ImGui::Text("Comparisons: %d", m_compareVisualizer2->getComparisons());
        ImGui::Text("Swaps: %d", m_compareVisualizer2->getSwaps());
        
        ImGui::NextColumn();
        
        // Analytical comparison
        ImGui::Text("Comparative Analytics Verdict:");
        ImGui::Separator();
        bool running1 = m_compareVisualizer1->isSorting();
        bool running2 = m_compareVisualizer2->isSorting();
        if (running1 || running2) {
            ImGui::Text("Threads active. Simulating running space complexity and sorting coefficients...");
        } else {
            float t1 = m_compareVisualizer1->getElapsedTime();
            float t2 = m_compareVisualizer2->getElapsedTime();
            if (t1 > 0.f && t2 > 0.f) {
                if (t1 < t2) {
                    float ratio = (t1 > 0.f) ? (t2 / t1) : 1.f;
                    ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().success), "» %s is %.1fx FASTER than %s!", sorts[m_compareAlgo1], ratio, sorts[m_compareAlgo2]);
                } else if (t2 < t1) {
                    float ratio = (t2 > 0.f) ? (t1 / t2) : 1.f;
                    ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().success), "» %s is %.1fx FASTER than %s!", sorts[m_compareAlgo2], ratio, sorts[m_compareAlgo1]);
                } else {
                    ImGui::Text("Runtimes are identical.");
                }
            } else {
                ImGui::Text("Launch comparison using 'Compare Run'.");
            }
        }
        ImGui::Columns(1);
    } else {
        switch (m_currentTab) {
            case ActiveTab::Sorting:
                m_sortingVisualizer->renderUI();
                break;
            case ActiveTab::Searching:
                m_searchingVisualizer->renderUI();
                break;
            case ActiveTab::Graph:
                m_graphVisualizer->renderUI();
                break;
            case ActiveTab::Pathfinding:
                m_pathfindingVisualizer->renderUI();
                break;
            case ActiveTab::DataStructures:
                m_dsVisualizer->renderUI();
                break;
            default:
                break;
        }
    }

    ImGui::End();
}

void App::drawMainWorkspace() {
    float sidebarWidth = 220.f;
    float topbarHeight = 55.f;
    float bottomPanelHeight = 220.f;
    
    ImGui::SetNextWindowPos(ImVec2(sidebarWidth, topbarHeight));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_window.getSize().x) - sidebarWidth, static_cast<float>(m_window.getSize().y) - topbarHeight - bottomPanelHeight));
    
    ImGui::Begin("Workspace Controls", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    
    if (m_sideBySideMode) {
        const char* sorts[] = { "Bubble Sort", "Selection Sort", "Insertion Sort", "Merge Sort", "Quick Sort", "Heap Sort", "Counting Sort", "Radix Sort" };
        
        ImGui::Text("Left Algo:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        ImGui::Combo("##LeftAlgoCombo", &m_compareAlgo1, sorts, IM_ARRAYSIZE(sorts));
        
        ImGui::SameLine();
        ImGui::Text("Right Algo:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        ImGui::Combo("##RightAlgoCombo", &m_compareAlgo2, sorts, IM_ARRAYSIZE(sorts));
        
        ImGui::SameLine();
        ImGui::SetNextItemWidth(130);
        ImGui::SliderInt("Size", &m_compareSize, 10, 180);

        ImGui::SameLine();
        if (ImGui::Button("Compare Run", ImVec2(100, 25))) {
            m_compareVisualizer1->startExternalSort(m_compareAlgo1, m_compareSize);
            m_compareVisualizer2->startExternalSort(m_compareAlgo2, m_compareSize);
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Reset Compare", ImVec2(100, 25))) {
            m_compareVisualizer1->reset();
            m_compareVisualizer2->reset();
        }
    }
    
    ImGui::End();
}

void App::render() {
    m_window.clear(ThemeManager::getInstance().getCurrentTheme().bg);

    // 1. Draw UI (this updates the ImGui draw buffers)
    renderUI();

    float sidebarWidth = 220.f;
    float topbarHeight = 55.f;
    float bottomPanelHeight = 220.f;

    // 2. Render SFML workspace graphics
    if (!m_sideBySideMode) {
        switch (m_currentTab) {
            case ActiveTab::Sorting:
                m_sortingVisualizer->render(m_window);
                break;
            case ActiveTab::Searching:
                m_searchingVisualizer->render(m_window);
                break;
            case ActiveTab::Graph:
                m_graphVisualizer->render(m_window);
                break;
            case ActiveTab::Pathfinding:
                m_pathfindingVisualizer->render(m_window);
                break;
            case ActiveTab::DataStructures:
                m_dsVisualizer->render(m_window);
                break;
            default:
                break;
        }
    } else {
        float windowWidth = static_cast<float>(m_window.getSize().x);
        float windowHeight = static_cast<float>(m_window.getSize().y);
        
        // Split the visualization canvas area horizontally into two panels
        float yStart = topbarHeight + 40.f;
        float width = (windowWidth - sidebarWidth - 30.f) / 2.f;
        float height = windowHeight - topbarHeight - bottomPanelHeight - 50.f;
        
        // Left visualization panel
        float xStartLeft = sidebarWidth + 10.f;
        m_compareVisualizer1->renderInBounds(m_window, xStartLeft, yStart, width, height);
        
        // Right visualization panel
        float xStartRight = sidebarWidth + 20.f + width;
        m_compareVisualizer2->renderInBounds(m_window, xStartRight, yStart, width, height);
    }

    // 3. Render ImGui on top
    ImGui::SFML::Render(m_window);

    m_window.display();
}
