#include "PathfindingVisualizer.hpp"
#include "../ui/ThemeManager.hpp"
#include <imgui.h>
#include <queue>
#include <stack>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>

PathfindingVisualizer::PathfindingVisualizer()
    : m_rows(20)
    , m_cols(38)
    , m_cellSize(25.f)
    , m_startX(4)
    , m_startY(10)
    , m_endX(32)
    , m_endY(10)
    , m_settingStart(false)
    , m_settingEnd(false)
    , m_drawingWalls(false)
    , m_clearingWalls(false)
    , m_running(false)
    , m_paused(false)
    , m_stepRequested(false)
    , m_delayMs(15)
    , m_visitedCount(0)
    , m_pathLength(0)
    , m_elapsedTime(0.f)
    , m_selectedAlgo(0) {

    m_algoNames = {
        "A* Search",
        "Dijkstra Pathfinder",
        "BFS Pathfinder"
    };

    initGrid();
}

PathfindingVisualizer::~PathfindingVisualizer() {
    stopAlgorithm();
}

void PathfindingVisualizer::initGrid() {
    stopAlgorithm();
    std::lock_guard<std::mutex> lock(m_gridMutex);

    m_grid.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_grid[r].resize(m_cols);
        for (int c = 0; c < m_cols; ++c) {
            m_grid[r][c].x = c;
            m_grid[r][c].y = r;
            m_grid[r][c].type = CellType::Empty;
        }
    }

    // Set defaults
    m_grid[m_startY][m_startX].type = CellType::Start;
    m_grid[m_endY][m_endX].type = CellType::End;

    m_visitedCount = 0;
    m_pathLength = 0;
    m_elapsedTime = 0.f;
}

void PathfindingVisualizer::startAlgorithm() {
    stopAlgorithm();
    m_running = true;
    m_paused = false;
    m_stepRequested = false;
    m_visitedCount = 0;
    m_pathLength = 0;
    m_elapsedTime = 0.f;
    m_timerClock.restart();

    // Reset path states (keep start, end, walls)
    std::lock_guard<std::mutex> lock(m_gridMutex);
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].type == CellType::Open || 
                m_grid[r][c].type == CellType::Closed || 
                m_grid[r][c].type == CellType::Path) {
                m_grid[r][c].type = CellType::Empty;
            }
        }
    }

    m_workerThread = std::thread(&PathfindingVisualizer::runAlgorithmLoop, this);
}

void PathfindingVisualizer::stopAlgorithm() {
    m_running = false;
    m_paused = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void PathfindingVisualizer::waitCheck() {
    if (!m_running) {
        throw std::runtime_error("Interrupted");
    }

    while (m_paused && m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (m_stepRequested) {
            m_stepRequested = false;
            return;
        }
    }

    if (m_delayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_delayMs));
    }
}

void PathfindingVisualizer::runAlgorithmLoop() {
    try {
        switch (m_selectedAlgo) {
            case 0: runAStar(); break;
            case 1: runDijkstra(); break;
            case 2: runBFS(); break;
        }
        m_running = false;
    }
    catch (const std::exception&) {
        m_running = false;
    }
}

std::vector<GridCell*> PathfindingVisualizer::getNeighbors(int cx, int cy) {
    std::vector<GridCell*> neighbors;
    // 4 directions (up, down, left, right)
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = cx + dx[i];
        int ny = cy + dy[i];

        if (nx >= 0 && nx < m_cols && ny >= 0 && ny < m_rows) {
            if (m_grid[ny][nx].type != CellType::Wall) {
                neighbors.push_back(&m_grid[ny][nx]);
            }
        }
    }
    return neighbors;
}

float PathfindingVisualizer::heuristic(int x1, int y1, int x2, int y2) const {
    // Manhattan distance
    return static_cast<float>(std::abs(x1 - x2) + std::abs(y1 - y2));
}

void PathfindingVisualizer::reconstructPath(const std::map<GridCell*, GridCell*>& parentMap, GridCell* current) {
    GridCell* temp = current;
    std::vector<GridCell*> path;
    
    while (parentMap.find(temp) != parentMap.end()) {
        temp = parentMap.at(temp);
        if (temp->type != CellType::Start) {
            path.push_back(temp);
        }
    }

    std::reverse(path.begin(), path.end());
    m_pathLength = path.size() + 1;

    for (auto* cell : path) {
        waitCheck();
        std::lock_guard<std::mutex> lock(m_gridMutex);
        cell->type = CellType::Path;
    }
}

void PathfindingVisualizer::handleEvent(const sf::Event& event) {
    if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseButtonPressed->position.x), static_cast<float>(mouseButtonPressed->position.y));

        // Canvas offsets
        float xStart = 240.f;
        float yStart = 75.f;

        // Check if mouse inside grid
        int col = static_cast<int>((mousePos.x - xStart) / m_cellSize);
        int row = static_cast<int>((mousePos.y - yStart) / m_cellSize);

        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            std::lock_guard<std::mutex> lock(m_gridMutex);
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                if (row == m_startY && col == m_startX) {
                    m_settingStart = true;
                } else if (row == m_endY && col == m_endX) {
                    m_settingEnd = true;
                } else {
                    if (m_grid[row][col].type == CellType::Wall) {
                        m_clearingWalls = true;
                        m_grid[row][col].type = CellType::Empty;
                    } else {
                        m_drawingWalls = true;
                        m_grid[row][col].type = CellType::Wall;
                    }
                }
            }
        }
    }
    else if (const auto* mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButtonReleased->button == sf::Mouse::Button::Left) {
            m_settingStart = false;
            m_settingEnd = false;
            m_drawingWalls = false;
            m_clearingWalls = false;
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y));

        float xStart = 240.f;
        float yStart = 75.f;

        int col = static_cast<int>((mousePos.x - xStart) / m_cellSize);
        int row = static_cast<int>((mousePos.y - yStart) / m_cellSize);

        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            std::lock_guard<std::mutex> lock(m_gridMutex);
            if (m_settingStart && (row != m_endY || col != m_endX)) {
                m_grid[m_startY][m_startX].type = CellType::Empty;
                m_startX = col;
                m_startY = row;
                m_grid[m_startY][m_startX].type = CellType::Start;
            } 
            else if (m_settingEnd && (row != m_startY || col != m_startX)) {
                m_grid[m_endY][m_endX].type = CellType::Empty;
                m_endX = col;
                m_endY = row;
                m_grid[m_endY][m_endX].type = CellType::End;
            } 
            else if (m_drawingWalls && (row != m_startY || col != m_startX) && (row != m_endY || col != m_endX)) {
                m_grid[row][col].type = CellType::Wall;
            } 
            else if (m_clearingWalls && (row != m_startY || col != m_startX) && (row != m_endY || col != m_endX)) {
                m_grid[row][col].type = CellType::Empty;
            }
        }
    }
}

void PathfindingVisualizer::update(float) {
    if (m_running && !m_paused) {
        m_elapsedTime = m_timerClock.getElapsedTime().asSeconds();
    }
}

void PathfindingVisualizer::render(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_gridMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    float xStart = 240.f;
    float yStart = 75.f;

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            sf::RectangleShape cell(sf::Vector2f(m_cellSize - 1.f, m_cellSize - 1.f));
            cell.setPosition({xStart + c * m_cellSize, yStart + r * m_cellSize});

            switch (m_grid[r][c].type) {
                case CellType::Empty:
                    cell.setFillColor(theme.inactive);
                    cell.setOutlineColor(theme.border);
                    cell.setOutlineThickness(0.5f);
                    break;
                case CellType::Wall:
                    cell.setFillColor(theme.normal);
                    break;
                case CellType::Start:
                    cell.setFillColor(theme.success); // Green
                    break;
                case CellType::End:
                    cell.setFillColor(theme.danger); // Red
                    break;
                case CellType::Open:
                    cell.setFillColor(theme.primary); // Blue
                    break;
                case CellType::Closed:
                    cell.setFillColor(theme.auxiliary); // Purple/Secondary
                    break;
                case CellType::Path:
                    cell.setFillColor(theme.secondary); // Yellow / Path
                    break;
            }
            window.draw(cell);
        }
    }
}

void PathfindingVisualizer::reset() {
    stopAlgorithm();
    initGrid();
}

// ==========================================
// PATHFINDING ALGORITHMS (Threaded)
// ==========================================

void PathfindingVisualizer::runAStar() {
    std::lock_guard<std::mutex> lock(m_gridMutex);
    GridCell* start = &m_grid[m_startY][m_startX];
    GridCell* end = &m_grid[m_endY][m_endX];

    // Priority Queue node wrapper
    struct PQNode {
        GridCell* cell;
        float fScore;
        bool operator>(const PQNode& other) const { return fScore > other.fScore; }
    };

    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> openSet;
    openSet.push({start, 0.f});

    std::map<GridCell*, GridCell*> parentMap;
    std::map<GridCell*, float> gScore;
    gScore[start] = 0.f;

    std::map<GridCell*, float> fScore;
    fScore[start] = heuristic(m_startX, m_startY, m_endX, m_endY);

    std::map<GridCell*, bool> openSetLookup;
    openSetLookup[start] = true;

    while (!openSet.empty()) {
        GridCell* current = openSet.top().cell;
        openSet.pop();
        openSetLookup[current] = false;

        if (current == end) {
            reconstructPath(parentMap, end);
            return;
        }

        if (current != start && current != end) {
            current->type = CellType::Closed;
        }
        m_visitedCount++;

        m_gridMutex.unlock();
        waitCheck();
        m_gridMutex.lock();

        for (auto* neighbor : getNeighbors(current->x, current->y)) {
            float tempG = gScore[current] + 1.f; // Distance is always 1 in grid

            if (gScore.find(neighbor) == gScore.end() || tempG < gScore[neighbor]) {
                parentMap[neighbor] = current;
                gScore[neighbor] = tempG;
                fScore[neighbor] = tempG + heuristic(neighbor->x, neighbor->y, m_endX, m_endY);

                if (!openSetLookup[neighbor]) {
                    openSet.push({neighbor, fScore[neighbor]});
                    openSetLookup[neighbor] = true;
                    if (neighbor != end) {
                        neighbor->type = CellType::Open;
                    }
                }
            }
        }
    }
}

void PathfindingVisualizer::runDijkstra() {
    std::lock_guard<std::mutex> lock(m_gridMutex);
    GridCell* start = &m_grid[m_startY][m_startX];
    GridCell* end = &m_grid[m_endY][m_endX];

    struct PQNode {
        GridCell* cell;
        float dist;
        bool operator>(const PQNode& other) const { return dist > other.dist; }
    };

    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> openSet;
    openSet.push({start, 0.f});

    std::map<GridCell*, GridCell*> parentMap;
    std::map<GridCell*, float> dist;
    dist[start] = 0.f;

    std::map<GridCell*, bool> openSetLookup;
    openSetLookup[start] = true;

    while (!openSet.empty()) {
        GridCell* current = openSet.top().cell;
        openSet.pop();
        openSetLookup[current] = false;

        if (current == end) {
            reconstructPath(parentMap, end);
            return;
        }

        if (current != start && current != end) {
            current->type = CellType::Closed;
        }
        m_visitedCount++;

        m_gridMutex.unlock();
        waitCheck();
        m_gridMutex.lock();

        for (auto* neighbor : getNeighbors(current->x, current->y)) {
            float tempD = dist[current] + 1.f;

            if (dist.find(neighbor) == dist.end() || tempD < dist[neighbor]) {
                parentMap[neighbor] = current;
                dist[neighbor] = tempD;

                if (!openSetLookup[neighbor]) {
                    openSet.push({neighbor, dist[neighbor]});
                    openSetLookup[neighbor] = true;
                    if (neighbor != end) {
                        neighbor->type = CellType::Open;
                    }
                }
            }
        }
    }
}

void PathfindingVisualizer::runBFS() {
    std::lock_guard<std::mutex> lock(m_gridMutex);
    GridCell* start = &m_grid[m_startY][m_startX];
    GridCell* end = &m_grid[m_endY][m_endX];

    std::queue<GridCell*> q;
    q.push(start);

    std::map<GridCell*, GridCell*> parentMap;
    std::map<GridCell*, bool> visited;
    visited[start] = true;

    while (!q.empty()) {
        GridCell* current = q.front();
        q.pop();

        if (current == end) {
            reconstructPath(parentMap, end);
            return;
        }

        if (current != start && current != end) {
            current->type = CellType::Closed;
        }
        m_visitedCount++;

        m_gridMutex.unlock();
        waitCheck();
        m_gridMutex.lock();

        for (auto* neighbor : getNeighbors(current->x, current->y)) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                parentMap[neighbor] = current;
                q.push(neighbor);
                if (neighbor != end) {
                    neighbor->type = CellType::Open;
                }
            }
        }
    }
}

// ==========================================
// PATHFINDING IMGUI CONTROLS
// ==========================================

void PathfindingVisualizer::renderUI() {
    ImGui::Columns(2, "PathfindingLayout", false);
    ImGui::SetColumnWidth(0, 480);

    ImGui::Text("Algorithm:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    if (ImGui::BeginCombo("##PathCombo", m_algoNames[m_selectedAlgo].c_str())) {
        for (int i = 0; i < static_cast<int>(m_algoNames.size()); i++) {
            bool isSelected = (m_selectedAlgo == i);
            if (ImGui::Selectable(m_algoNames[i].c_str(), isSelected)) {
                m_selectedAlgo = i;
                reset();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // Controls
    if (m_running) {
        if (m_paused) {
            if (ImGui::Button("Resume", ImVec2(80, 25))) m_paused = false;
            ImGui::SameLine();
            if (ImGui::Button("Step ⏵", ImVec2(80, 25))) m_stepRequested = true;
        } else {
            if (ImGui::Button("Pause", ImVec2(80, 25))) m_paused = true;
        }
    } else {
        if (ImGui::Button("Find Path", ImVec2(80, 25))) startAlgorithm();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Path", ImVec2(90, 25))) reset();
    ImGui::SameLine();
    if (ImGui::Button("Clear Walls", ImVec2(90, 25))) {
        reset();
    }

    ImGui::Spacing();
    ImGui::SetNextItemWidth(200);
    int delayVal = m_delayMs.load();
    if (ImGui::SliderInt("Speed Delay (ms)", &delayVal, 0, 150)) {
        m_delayMs = delayVal;
    }

    ImGui::NextColumn();

    // Metrics
    ImGui::Text("ANALYTICS");
    ImGui::Separator();

    ImGui::Columns(3, "PathfindingMetrics", false);
    ImGui::SetColumnWidth(0, 150);
    ImGui::SetColumnWidth(1, 150);
    ImGui::SetColumnWidth(2, 180);

    ImGui::Metric("Timer", "%.2fs", m_elapsedTime);
    ImGui::NextColumn();
    ImGui::Metric("Visited Cells", "%d", m_visitedCount.load());
    ImGui::NextColumn();
    ImGui::Metric("Path Cells", "%d", m_pathLength.load());

    ImGui::Columns(1);
    ImGui::Spacing();

    ImGui::TextDisabled("How to draw: [Left-Click and Drag] to paint walls. Drag the Green circle (Start) and Red circle (End) to move endpoints.");
    ImGui::Separator();

    if (m_selectedAlgo == 0) {
        ImGui::Text("A* Pathfinder: Evaluates f(n) = g(n) + h(n). Guaranteed shortest path while using Euclidean heuristics to speed up search.");
    } else if (m_selectedAlgo == 1) {
        ImGui::Text("Dijkstra: Single-source shortest path on grids. Evaluates cells based purely on distance from start. Equivalent to BFS on uniform cells.");
    } else if (m_selectedAlgo == 2) {
        ImGui::Text("BFS: Explores frontier layer-by-layer uniformly. Guarantees shortest path on unweighted grids.");
    }
}
