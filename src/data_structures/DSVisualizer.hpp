#pragma once
#include "../VisualizerModule.hpp"
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

// Node structures for visualizations
struct DSNode {
    int value;
    sf::Vector2f position;
    sf::Vector2f targetPosition;
    sf::Color color;
    float alpha = 255.f; // For fade effects
};

struct BSTNode {
    int value;
    sf::Vector2f position;
    std::shared_ptr<BSTNode> left = nullptr;
    std::shared_ptr<BSTNode> right = nullptr;
    sf::Color color;
    bool isTraversed = false;
};

enum class DSType {
    Stack,
    Queue,
    LinkedList,
    BST,
    Heap
};

class DSVisualizer : public VisualizerModule {
public:
    DSVisualizer();
    ~DSVisualizer() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void renderUI() override;
    void reset() override;

private:
    void updateAnimations(float dt);
    void startThreadAction();
    void stopThreadAction();
    void waitCheck();

    // Visualizers per type
    void renderStack(sf::RenderWindow& window);
    void renderQueue(sf::RenderWindow& window);
    void renderLinkedList(sf::RenderWindow& window);
    void renderBST(sf::RenderWindow& window);
    void renderBSTHelper(sf::RenderWindow& window, std::shared_ptr<BSTNode> node, sf::Vector2f parentPos);
    void renderHeap(sf::RenderWindow& window);

    // Operations (with optional animated execution threads)
    void executePush(int val);
    void executePop();
    void executeEnqueue(int val);
    void executeDequeue();
    
    void executeListInsert(int val, int index);
    void executeListDelete(int val);
    void executeListSearch(int val);

    void executeBSTInsert(int val);
    void executeBSTDelete(int val);
    void executeBSTTraversal(int type); // 0: In, 1: Pre, 2: Post
    void animateBSTInsert(std::shared_ptr<BSTNode>& node, int val, sf::Vector2f pos, float offset);
    void animateBSTTraversal(std::shared_ptr<BSTNode> node, std::vector<std::shared_ptr<BSTNode>>& order);

    void executeHeapInsert(int val);
    void executeHeapDelete();
    void heapifyUp(int index);
    void heapifyDown(int index);

    // Positions calculations
    void recalculateBSTPositions(std::shared_ptr<BSTNode> node, float x, float y, float xOffset);

    // Data Structures Storage
    std::vector<DSNode> m_linearData; // For Stack, Queue, Linked List
    std::shared_ptr<BSTNode> m_bstRoot; // For BST
    std::vector<DSNode> m_heapData;   // For Heap (represented as array first, rendered as tree)

    DSType m_currentDS;
    int m_inputValue;
    int m_inputIndex;

    // Execution Thread
    std::thread m_workerThread;
    std::mutex m_dsMutex;
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::atomic<int> m_delayMs;
    
    // Status logs
    std::string m_operationStatus;
    sf::Font m_font;
};
