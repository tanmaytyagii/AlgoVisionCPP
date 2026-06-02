#include "GraphVisualizer.hpp"
#include "../ui/ThemeManager.hpp"
#include <imgui.h>
#include <cstdint>
#include <cmath>
#include <queue>
#include <stack>
#include <algorithm>
#include <iostream>

GraphVisualizer::GraphVisualizer()
    : m_nextNodeId(0)
    , m_edgeStartNode(-1)
    , m_drawingEdge(false)
    , m_mouseMode(MouseMode::SelectAndDrag)
    , m_activeNodeId(-1)
    , m_startNodeId(0)
    , m_endNodeId(4)
    , m_running(false)
    , m_paused(false)
    , m_stepRequested(false)
    , m_delayMs(500)
    , m_visitedCount(0)
    , m_comparisons(0)
    , m_elapsedTime(0.f)
    , m_selectedAlgo(0)
    , m_newEdgeWeight(5.f) {

    m_algoNames = {
        "BFS Traversal",
        "DFS Traversal",
        "Dijkstra Shortest Path",
        "Kruskal's MST",
        "Prim's MST"
    };

    // Load default font for edge weights
    if (!m_font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        // Fallback or ignore
    }

    loadSampleGraph();
}

GraphVisualizer::~GraphVisualizer() {
    stopAlgorithm();
}

void GraphVisualizer::loadSampleGraph() {
    clearGraph();
    std::lock_guard<std::mutex> lock(m_graphMutex);

    // Create 5 sample nodes in a circle-ish pattern
    addNode({380.f, 250.f}); // 0
    addNode({580.f, 150.f}); // 1
    addNode({780.f, 250.f}); // 2
    addNode({680.f, 450.f}); // 3
    addNode({480.f, 450.f}); // 4

    // Connect them with weighted edges
    addEdge(0, 1, 4.f);
    addEdge(0, 4, 8.f);
    addEdge(1, 2, 3.f);
    addEdge(1, 4, 1.f);
    addEdge(2, 3, 2.f);
    addEdge(3, 4, 7.f);
    addEdge(1, 3, 5.f);

    m_startNodeId = 0;
    m_endNodeId = 3;
}

void GraphVisualizer::clearGraph() {
    stopAlgorithm();
    std::lock_guard<std::mutex> lock(m_graphMutex);
    m_nodes.clear();
    m_edges.clear();
    m_nextNodeId = 0;
    m_startNodeId = 0;
    m_endNodeId = 0;
    m_visitedCount = 0;
    m_comparisons = 0;
    m_elapsedTime = 0.f;
}

void GraphVisualizer::addNode(sf::Vector2f pos) {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    GraphNode node;
    node.id = m_nextNodeId++;
    node.position = pos;
    node.color = theme.normal;
    m_nodes.push_back(node);
}

void GraphVisualizer::addEdge(int u, int v, float weight) {
    // Check if edge already exists
    for (auto& edge : m_edges) {
        if ((edge.u == u && edge.v == v) || (edge.u == v && edge.v == u)) {
            edge.weight = weight; // Update weight
            return;
        }
    }

    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    GraphEdge edge;
    edge.u = u;
    edge.v = v;
    edge.weight = weight;
    edge.color = theme.border;
    m_edges.push_back(edge);
}

int GraphVisualizer::findNodeAt(sf::Vector2f pos) const {
    const float radius = 22.f;
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        float dist = std::hypot(pos.x - m_nodes[i].position.x, pos.y - m_nodes[i].position.y);
        if (dist <= radius) {
            return m_nodes[i].id;
        }
    }
    return -1;
}

void GraphVisualizer::startAlgorithm() {
    stopAlgorithm();
    m_running = true;
    m_paused = false;
    m_stepRequested = false;
    m_visitedCount = 0;
    m_comparisons = 0;
    m_elapsedTime = 0.f;
    m_timerClock.restart();

    // Reset node & edge colors before run
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (auto& node : m_nodes) {
        node.color = theme.normal;
        node.pulseRadius = 0.f;
    }
    for (auto& edge : m_edges) {
        edge.color = theme.border;
        edge.isHighlighted = false;
        edge.isMST = false;
    }

    m_workerThread = std::thread(&GraphVisualizer::runAlgorithmLoop, this);
}

void GraphVisualizer::stopAlgorithm() {
    m_running = false;
    m_paused = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void GraphVisualizer::waitCheck() {
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

void GraphVisualizer::runAlgorithmLoop() {
    try {
        switch (m_selectedAlgo) {
            case 0: runBFS(); break;
            case 1: runDFS(); break;
            case 2: runDijkstra(); break;
            case 3: runKruskal(); break;
            case 4: runPrim(); break;
        }
        m_running = false;
    }
    catch (const std::exception&) {
        m_running = false;
    }
}

void GraphVisualizer::handleEvent(const sf::Event& event) {
    if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseButtonPressed->position.x), static_cast<float>(mouseButtonPressed->position.y));

        if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
            int nodeUnder = findNodeAt(mousePos);

            if (m_mouseMode == MouseMode::SelectAndDrag) {
                if (nodeUnder != -1) {
                    m_activeNodeId = nodeUnder;
                    std::lock_guard<std::mutex> lock(m_graphMutex);
                    for (auto& node : m_nodes) {
                        if (node.id == m_activeNodeId) {
                            node.isDragging = true;
                        }
                    }
                }
            } 
            else if (m_mouseMode == MouseMode::AddNode) {
                if (nodeUnder == -1) {
                    std::lock_guard<std::mutex> lock(m_graphMutex);
                    addNode(mousePos);
                }
            } 
            else if (m_mouseMode == MouseMode::AddEdge) {
                if (nodeUnder != -1) {
                    m_drawingEdge = true;
                    m_edgeStartNode = nodeUnder;
                    m_tempEdgeEnd = mousePos;
                }
            }
        }
        else if (mouseButtonPressed->button == sf::Mouse::Button::Right) {
            // Delete node or edge
            int nodeUnder = findNodeAt(mousePos);
            std::lock_guard<std::mutex> lock(m_graphMutex);
            if (nodeUnder != -1) {
                // Delete node
                m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(), [nodeUnder](const GraphNode& n) {
                    return n.id == nodeUnder;
                }), m_nodes.end());
                // Delete connected edges
                m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), [nodeUnder](const GraphEdge& e) {
                    return e.u == nodeUnder || e.v == nodeUnder;
                }), m_edges.end());
            }
        }
    }
    else if (const auto* mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButtonReleased->button == sf::Mouse::Button::Left) {
            if (m_activeNodeId != -1) {
                std::lock_guard<std::mutex> lock(m_graphMutex);
                for (auto& node : m_nodes) {
                    if (node.id == m_activeNodeId) {
                        node.isDragging = false;
                    }
                }
                m_activeNodeId = -1;
            }

            if (m_drawingEdge) {
                sf::Vector2f mousePos(static_cast<float>(mouseButtonReleased->position.x), static_cast<float>(mouseButtonReleased->position.y));
                int nodeUnder = findNodeAt(mousePos);
                if (nodeUnder != -1 && nodeUnder != m_edgeStartNode) {
                    std::lock_guard<std::mutex> lock(m_graphMutex);
                    addEdge(m_edgeStartNode, nodeUnder, m_newEdgeWeight);
                }
                m_drawingEdge = false;
                m_edgeStartNode = -1;
            }
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y));

        if (m_activeNodeId != -1) {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == m_activeNodeId) {
                    node.position = mousePos;
                }
            }
        }

        if (m_drawingEdge) {
            m_tempEdgeEnd = mousePos;
        }
    }
}

void GraphVisualizer::update(float dt) {
    if (m_running && !m_paused) {
        m_elapsedTime = m_timerClock.getElapsedTime().asSeconds();
    }

    // Dynamic pulse animations for visited nodes
    std::lock_guard<std::mutex> lock(m_graphMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (auto& node : m_nodes) {
        if (node.color == theme.success || node.color == theme.primary) {
            node.pulseRadius += dt * 30.f;
            if (node.pulseRadius > 35.f) node.pulseRadius = 22.f;
        } else {
            node.pulseRadius = 0.f;
        }
    }
}

void GraphVisualizer::render(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    // 1. Draw Edges
    for (const auto& edge : m_edges) {
        // Find nodes endpoints
        sf::Vector2f p1, p2;
        bool uFound = false, vFound = false;
        for (const auto& node : m_nodes) {
            if (node.id == edge.u) { p1 = node.position; uFound = true; }
            if (node.id == edge.v) { p2 = node.position; vFound = true; }
        }

        if (uFound && vFound) {
            // Draw line
            sf::Vertex line[] = {
                sf::Vertex{p1, edge.color},
                sf::Vertex{p2, edge.color}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);

            // Draw Edge Weight Text
            sf::Vector2f mid = (p1 + p2) / 2.f;
            
            // Draw a tiny background rect for weight text
            sf::RectangleShape labelBg(sf::Vector2f(22.f, 16.f));
            labelBg.setOrigin({11.f, 8.f});
            labelBg.setPosition(mid);
            labelBg.setFillColor(theme.bg);
            labelBg.setOutlineThickness(1.f);
            labelBg.setOutlineColor(theme.border);
            window.draw(labelBg);

            // Edge weight value
            sf::Text weightText(m_font, std::to_string(static_cast<int>(edge.weight)), 12);
            weightText.setFillColor(theme.text);
            sf::FloatRect bounds = weightText.getLocalBounds();
            weightText.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 2.f});
            weightText.setPosition(mid);
            window.draw(weightText);
        }
    }

    // 2. Draw Temporary Edge if drawing
    if (m_drawingEdge && m_edgeStartNode != -1) {
        sf::Vector2f startPos;
        for (const auto& node : m_nodes) {
            if (node.id == m_edgeStartNode) startPos = node.position;
        }
        sf::Vertex tempLine[] = {
            sf::Vertex{startPos, theme.primary},
            sf::Vertex{m_tempEdgeEnd, theme.primary}
        };
        window.draw(tempLine, 2, sf::PrimitiveType::Lines);
    }

    // 3. Draw Nodes
    const float radius = 22.f;
    for (const auto& node : m_nodes) {
        // Pulse animation circle
        if (node.pulseRadius > radius) {
            sf::CircleShape pulse(node.pulseRadius);
            pulse.setOrigin({node.pulseRadius, node.pulseRadius});
            pulse.setPosition(node.position);
            sf::Color pColor = theme.primary;
            pColor.a = static_cast<std::uint8_t>(255 * (1.f - (node.pulseRadius - radius) / (35.f - radius)));
            pulse.setFillColor(sf::Color::Transparent);
            pulse.setOutlineThickness(1.5f);
            pulse.setOutlineColor(pColor);
            window.draw(pulse);
        }

        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(node.position);
        circle.setFillColor(node.color);
        circle.setOutlineThickness(2.f);
        
        // Highlight Start and End nodes with thicker borders
        if (node.id == m_startNodeId) {
            circle.setOutlineColor(theme.success);
        } else if (node.id == m_endNodeId && (m_selectedAlgo == 0 || m_selectedAlgo == 1 || m_selectedAlgo == 2)) {
            circle.setOutlineColor(theme.danger);
        } else {
            circle.setOutlineColor(theme.border);
        }
        window.draw(circle);

        // Node ID label
        sf::Text text(m_font, std::to_string(node.id), 14);
        text.setFillColor(theme.text);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
        text.setPosition(node.position);
        window.draw(text);
    }
}

void GraphVisualizer::reset() {
    stopAlgorithm();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    std::lock_guard<std::mutex> lock(m_graphMutex);
    for (auto& node : m_nodes) {
        node.color = theme.normal;
    }
    for (auto& edge : m_edges) {
        edge.color = theme.border;
        edge.isHighlighted = false;
        edge.isMST = false;
    }
    m_visitedCount = 0;
    m_comparisons = 0;
    m_elapsedTime = 0.f;
}

// ==========================================
// GRAPH TRAVERSALS & ALGORITHMS (Threaded)
// ==========================================

void GraphVisualizer::runBFS() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    std::queue<int> q;
    std::vector<bool> visited(m_nextNodeId, false);

    q.push(m_startNodeId);
    visited[m_startNodeId] = true;

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == curr) node.color = theme.primary; // Blue: current active
            }
        }
        m_visitedCount++;

        // Visit neighbors
        std::vector<int> neighbors;
        for (const auto& edge : m_edges) {
            int adj = -1;
            if (edge.u == curr) adj = edge.v;
            else if (edge.v == curr) adj = edge.u;

            if (adj != -1 && !visited[adj]) {
                neighbors.push_back(adj);
                visited[adj] = true;
                q.push(adj);

                waitCheck();
                {
                    std::lock_guard<std::mutex> lock(m_graphMutex);
                    // Color the traversed edge
                    for (auto& e : m_edges) {
                        if ((e.u == curr && e.v == adj) || (e.v == curr && e.u == adj)) {
                            e.color = theme.primary;
                        }
                    }
                    for (auto& node : m_nodes) {
                        if (node.id == adj) node.color = theme.secondary; // Yellow: in queue frontier
                    }
                }
            }
        }

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == curr) node.color = theme.success; // Green: explored
            }
        }
        
        if (curr == m_endNodeId) return; // Shortest path pathfound
    }
}

void GraphVisualizer::runDFS() {
    std::vector<bool> visited(m_nextNodeId, false);
    runDFS_Helper(m_startNodeId, visited);
}

void GraphVisualizer::runDFS_Helper(int curr, std::vector<bool>& visited) {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    visited[curr] = true;

    waitCheck();
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        for (auto& node : m_nodes) {
            if (node.id == curr) node.color = theme.primary; // Blue: active
        }
    }
    m_visitedCount++;

    if (curr == m_endNodeId) {
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == curr) node.color = theme.success;
            }
        }
        throw std::runtime_error("Found End"); // Unwind DFS recurse
    }

    // Get adjacent nodes
    std::vector<std::pair<int, int>> neighbors; // {adj_node_id, edge_index}
    for (size_t i = 0; i < m_edges.size(); ++i) {
        if (m_edges[i].u == curr && !visited[m_edges[i].v]) {
            neighbors.push_back({m_edges[i].v, static_cast<int>(i)});
        }
        else if (m_edges[i].v == curr && !visited[m_edges[i].u]) {
            neighbors.push_back({m_edges[i].u, static_cast<int>(i)});
        }
    }

    for (const auto& neighbor : neighbors) {
        int adj = neighbor.first;
        int edgeIdx = neighbor.second;

        if (!visited[adj]) {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_graphMutex);
                m_edges[edgeIdx].color = theme.primary;
            }

            runDFS_Helper(adj, visited);
        }
    }

    waitCheck();
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        for (auto& node : m_nodes) {
            if (node.id == curr) node.color = theme.success; // Green: fully finished
        }
    }
}

void GraphVisualizer::runDijkstra() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    
    std::vector<float> dist(m_nextNodeId, 1e9f);
    std::vector<int> parent(m_nextNodeId, -1);
    std::vector<bool> visited(m_nextNodeId, false);

    dist[m_startNodeId] = 0.f;

    for (int count = 0; count < m_nextNodeId; ++count) {
        // Find node with min distance
        float min = 1e9f;
        int u = -1;

        for (int i = 0; i < m_nextNodeId; ++i) {
            if (!visited[i] && dist[i] < min) {
                min = dist[i];
                u = i;
            }
        }

        if (u == -1) break;

        visited[u] = true;
        
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == u) node.color = theme.primary; // Blue: current min relaxation
            }
        }
        m_visitedCount++;

        // Relax neighbors
        for (const auto& edge : m_edges) {
            int v = -1;
            if (edge.u == u) v = edge.v;
            else if (edge.v == u) v = edge.u;

            if (v != -1 && !visited[v]) {
                m_comparisons++;
                if (dist[u] + edge.weight < dist[v]) {
                    dist[v] = dist[u] + edge.weight;
                    parent[v] = u;

                    waitCheck();
                    {
                        std::lock_guard<std::mutex> lock(m_graphMutex);
                        for (auto& node : m_nodes) {
                            if (node.id == v) node.color = theme.secondary; // yellow: potential updates
                        }
                    }
                }
            }
        }

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == u) node.color = theme.success; // Green: finalized
            }
        }

        if (u == m_endNodeId) break;
    }

    // Highlight final path
    int curr = m_endNodeId;
    while (curr != -1 && parent[curr] != -1) {
        int p = parent[curr];
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& edge : m_edges) {
                if ((edge.u == curr && edge.v == p) || (edge.v == curr && edge.u == p)) {
                    edge.color = theme.success; // Green for path edges
                    edge.isHighlighted = true;
                }
            }
            for (auto& node : m_nodes) {
                if (node.id == curr || node.id == p) {
                    node.color = theme.success;
                }
            }
        }
        curr = p;
    }
}

// Disjoint Set DSU helper for Kruskal's MST
struct DSU {
    std::vector<int> parent;
    DSU(int n) {
        parent.resize(n);
        for(int i = 0; i < n; ++i) parent[i] = i;
    }
    int find(int i) {
        if (parent[i] == i)
            return i;
        return parent[i] = find(parent[i]);
    }
    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            parent[root_i] = root_j;
            return true;
        }
        return false;
    }
};

void GraphVisualizer::runKruskal() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    
    // Sort edges
    std::vector<GraphEdge*> sortedEdges;
    for (auto& edge : m_edges) sortedEdges.push_back(&edge);

    std::sort(sortedEdges.begin(), sortedEdges.end(), [](const GraphEdge* a, const GraphEdge* b) {
        return a->weight < b->weight;
    });

    DSU dsu(m_nextNodeId);
    int mstEdges = 0;

    for (auto* edge : sortedEdges) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            edge->color = theme.danger; // Red: testing edge
        }
        m_comparisons++;

        if (dsu.unite(edge->u, edge->v)) {
            mstEdges++;
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_graphMutex);
                edge->color = theme.success; // Green: belongs to MST
                edge->isMST = true;
                // Highlight nodes in success color
                for (auto& n : m_nodes) {
                    if (n.id == edge->u || n.id == edge->v) n.color = theme.success;
                }
            }
            if (mstEdges == m_nextNodeId - 1) break;
        } else {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_graphMutex);
                edge->color = theme.inactive; // gray: cycle-forming discarded
            }
        }
    }
}

void GraphVisualizer::runPrim() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    std::vector<bool> inMST(m_nextNodeId, false);
    std::vector<float> key(m_nextNodeId, 1e9f);
    std::vector<int> parent(m_nextNodeId, -1);

    key[0] = 0.f;

    for (int count = 0; count < m_nextNodeId; ++count) {
        // Pick minimum key vertex not in MST
        float minKey = 1e9f;
        int u = -1;
        for (int i = 0; i < m_nextNodeId; ++i) {
            if (!inMST[i] && key[i] < minKey) {
                minKey = key[i];
                u = i;
            }
        }

        if (u == -1) break;

        inMST[u] = true;

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_graphMutex);
            for (auto& node : m_nodes) {
                if (node.id == u) node.color = theme.success; // add to MST
            }
            // Highlight MST edge
            if (parent[u] != -1) {
                for (auto& edge : m_edges) {
                    if ((edge.u == u && edge.v == parent[u]) || (edge.v == u && edge.u == parent[u])) {
                        edge.color = theme.success;
                        edge.isMST = true;
                    }
                }
            }
        }
        m_visitedCount++;

        // Update key value of adjacent vertices
        for (const auto& edge : m_edges) {
            int v = -1;
            if (edge.u == u) v = edge.v;
            else if (edge.v == u) v = edge.u;

            if (v != -1 && !inMST[v]) {
                m_comparisons++;
                if (edge.weight < key[v]) {
                    key[v] = edge.weight;
                    parent[v] = u;

                    waitCheck();
                    {
                        std::lock_guard<std::mutex> lock(m_graphMutex);
                        for (auto& node : m_nodes) {
                            if (node.id == v) node.color = theme.secondary; // yellow: relaxed edge frontier
                        }
                    }
                }
            }
        }
    }
}

// ==========================================
// GRAPH UI CONTROLS & LEARNING
// ==========================================

void GraphVisualizer::renderUI() {
    ImGui::Columns(2, "GraphLayout", false);
    ImGui::SetColumnWidth(0, 480);

    ImGui::Text("Algorithm:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    if (ImGui::BeginCombo("##GraphCombo", m_algoNames[m_selectedAlgo].c_str())) {
        for (int i = 0; i < static_cast<int>(m_algoNames.size()); i++) {
            bool isSelected = (m_selectedAlgo == i);
            if (ImGui::Selectable(m_algoNames[i].c_str(), isSelected)) {
                m_selectedAlgo = i;
                reset();
            }
        }
        ImGui::EndCombo();
    }

    // Modes & actions
    ImGui::Spacing();
    ImGui::Text("Mouse Mode:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Drag Nodes", m_mouseMode == MouseMode::SelectAndDrag)) m_mouseMode = MouseMode::SelectAndDrag;
    ImGui::SameLine();
    if (ImGui::RadioButton("+ Node", m_mouseMode == MouseMode::AddNode)) m_mouseMode = MouseMode::AddNode;
    ImGui::SameLine();
    if (ImGui::RadioButton("+ Edge", m_mouseMode == MouseMode::AddEdge)) m_mouseMode = MouseMode::AddEdge;

    ImGui::Spacing();
    if (m_running) {
        if (m_paused) {
            if (ImGui::Button("Resume", ImVec2(80, 25))) m_paused = false;
            ImGui::SameLine();
            if (ImGui::Button("Step ⏵", ImVec2(80, 25))) m_stepRequested = true;
        } else {
            if (ImGui::Button("Pause", ImVec2(80, 25))) m_paused = true;
        }
    } else {
        if (ImGui::Button("Run", ImVec2(80, 25))) startAlgorithm();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset Colors", ImVec2(100, 25))) reset();
    ImGui::SameLine();
    if (ImGui::Button("Load Sample", ImVec2(100, 25))) loadSampleGraph();
    ImGui::SameLine();
    if (ImGui::Button("Clear Graph", ImVec2(100, 25))) clearGraph();

    // Node bounds selectors
    ImGui::Spacing();
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Start Node", &m_startNodeId);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("End Node", &m_endNodeId);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::InputFloat("Edge Weight", &m_newEdgeWeight, 1.f, 5.f, "%.0f");

    ImGui::NextColumn();

    // Analytics
    ImGui::Text("ANALYTICS");
    ImGui::Separator();

    ImGui::Columns(3, "GraphMetrics2", false);
    ImGui::SetColumnWidth(0, 150);
    ImGui::SetColumnWidth(1, 150);
    ImGui::SetColumnWidth(2, 180);

    ImGui::Metric("Timer", "%.2fs", m_elapsedTime);
    ImGui::NextColumn();
    ImGui::Metric("Visited Nodes", "%d", m_visitedCount.load());
    ImGui::NextColumn();
    ImGui::Metric("Relaxation Steps", "%d", m_comparisons.load());

    ImGui::Columns(1);
    ImGui::Spacing();

    // Tips
    ImGui::TextDisabled("Controls: [Left-Click Canvas] to place node/edge. [Right-Click Node] to delete. [Drag] node circles to move.");
    ImGui::Separator();

    if (m_selectedAlgo == 0) {
        ImGui::Text("BFS: Breadth-First Search traversal explores level-by-level using a FIFO queue. Complexity: O(V + E).");
    } else if (m_selectedAlgo == 1) {
        ImGui::Text("DFS: Depth-First Search traversal explores deep paths using recursion (LIFO stack). Complexity: O(V + E).");
    } else if (m_selectedAlgo == 2) {
        ImGui::Text("Dijkstra: Single-source shortest path algorithm using a min-priority queue. Complexity: O(E log V).");
    } else if (m_selectedAlgo == 3) {
        ImGui::Text("Kruskal's: MST finder. Sorts edges and applies Union-Find (DSU). Complexity: O(E log E).");
    } else if (m_selectedAlgo == 4) {
        ImGui::Text("Prim's: MST finder. Grows tree from root using adjacent priority queue. Complexity: O(E log V).");
    }
}
