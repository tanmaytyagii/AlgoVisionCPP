#pragma once
#include "../VisualizerModule.hpp"
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

enum class CellType {
    Empty,
    Wall,
    Start,
    End,
    Open,
    Closed,
    Path
};

struct GridCell {
    int x;
    int y;
    CellType type = CellType::Empty;
};

class PathfindingVisualizer : public VisualizerModule {
public:
    PathfindingVisualizer();
    ~PathfindingVisualizer() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void renderUI() override;
    void reset() override;

private:
    void initGrid();
    void startAlgorithm();
    void stopAlgorithm();
    void waitCheck();
    void runAlgorithmLoop();

    // Pathfinding Algorithms
    void runAStar();
    void runDijkstra();
    void runBFS();

    // Helper functions
    std::vector<GridCell*> getNeighbors(int cx, int cy);
    float heuristic(int x1, int y1, int x2, int y2) const;
    void reconstructPath(const std::map<GridCell*, GridCell*>& parentMap, GridCell* current);

    // Grid Dimensions
    int m_rows;
    int m_cols;
    float m_cellSize;
    std::vector<std::vector<GridCell>> m_grid;

    // Start & End tracking
    int m_startX, m_startY;
    int m_endX, m_endY;
    bool m_settingStart;
    bool m_settingEnd;
    bool m_drawingWalls;
    bool m_clearingWalls;

    // Multithreading and flow control
    std::thread m_workerThread;
    std::mutex m_gridMutex;
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::atomic<bool> m_stepRequested;
    std::atomic<int> m_delayMs;

    // Metrics tracking
    std::atomic<int> m_visitedCount;
    std::atomic<int> m_pathLength;
    float m_elapsedTime;
    sf::Clock m_timerClock;

    int m_selectedAlgo;
    std::vector<std::string> m_algoNames;
};
