#pragma once
#include "../VisualizerModule.hpp"
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>

struct GraphNode {
    int id;
    sf::Vector2f position;
    bool isDragging = false;
    sf::Color color;
    float pulseRadius = 0.f; // For animations
};

struct GraphEdge {
    int u; // from node ID
    int v; // to node ID
    float weight;
    sf::Color color;
    bool isHighlighted = false;
    bool isMST = false;
};

class GraphVisualizer : public VisualizerModule {
public:
    GraphVisualizer();
    ~GraphVisualizer() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void renderUI() override;
    void reset() override;

private:
    // Core interaction modes
    enum class MouseMode {
        SelectAndDrag,
        AddNode,
        AddEdge
    };

    void startAlgorithm();
    void stopAlgorithm();
    void waitCheck();
    void runAlgorithmLoop();

    // Graph Algorithms
    void runBFS();
    void runDFS();
    void runDFS_Helper(int node, std::vector<bool>& visited);
    void runDijkstra();
    void runKruskal();
    void runPrim();

    // Node & Edge manipulation helpers
    int findNodeAt(sf::Vector2f pos) const;
    void addNode(sf::Vector2f pos);
    void addEdge(int u, int v, float weight);
    void clearGraph();
    void loadSampleGraph();

    // Graph data representation
    std::vector<GraphNode> m_nodes;
    std::vector<GraphEdge> m_edges;
    int m_nextNodeId;

    // Edge drawing tracking
    int m_edgeStartNode;
    sf::Vector2f m_tempEdgeEnd;
    bool m_drawingEdge;

    // Visual & State variables
    MouseMode m_mouseMode;
    int m_activeNodeId; // Currently dragged node
    int m_startNodeId;  // Dijkstra/BFS start node
    int m_endNodeId;    // Dijkstra/BFS target node
    
    // Algorithmic thread & synchronization
    std::thread m_workerThread;
    std::mutex m_graphMutex;
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::atomic<bool> m_stepRequested;
    std::atomic<int> m_delayMs;

    // Tracking metrics
    std::atomic<int> m_visitedCount;
    std::atomic<int> m_comparisons;
    float m_elapsedTime;
    sf::Clock m_timerClock;

    int m_selectedAlgo;
    std::vector<std::string> m_algoNames;
    float m_newEdgeWeight;
    sf::Font m_font;
};
