#include "DSVisualizer.hpp"
#include "../ui/ThemeManager.hpp"
#include <imgui.h>
#include <cmath>
#include <iostream>
#include <algorithm>

DSVisualizer::DSVisualizer()
    : m_bstRoot(nullptr)
    , m_currentDS(DSType::Stack)
    , m_inputValue(15)
    , m_inputIndex(0)
    , m_running(false)
    , m_paused(false)
    , m_delayMs(600)
    , m_operationStatus("Ready") {
    
    if (!m_font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        // Fallback or ignore
    }

    reset();
}

DSVisualizer::~DSVisualizer() {
    stopThreadAction();
}

void DSVisualizer::reset() {
    stopThreadAction();
    std::lock_guard<std::mutex> lock(m_dsMutex);
    
    m_linearData.clear();
    m_bstRoot = nullptr;
    m_heapData.clear();
    
    m_operationStatus = "Ready";

    // Load default datasets
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    if (m_currentDS == DSType::Stack || m_currentDS == DSType::Queue || m_currentDS == DSType::LinkedList) {
        std::vector<int> defaults = {10, 20, 30, 40};
        for (int val : defaults) {
            DSNode node;
            node.value = val;
            node.color = theme.normal;
            node.position = {0.f, 0.f};
            node.targetPosition = {0.f, 0.f};
            m_linearData.push_back(node);
        }
    } 
    else if (m_currentDS == DSType::BST) {
        std::vector<int> defaults = {30, 15, 45, 7, 22, 35, 60};
        for (int val : defaults) {
            // Simple synchronous insertion
            executeBSTInsert(val);
        }
    } 
    else if (m_currentDS == DSType::Heap) {
        std::vector<int> defaults = {80, 50, 40, 20, 10, 30};
        for (int val : defaults) {
            DSNode node;
            node.value = val;
            node.color = theme.normal;
            m_heapData.push_back(node);
        }
    }
}

void DSVisualizer::startThreadAction() {
    stopThreadAction();
    m_running = true;
    m_paused = false;
}

void DSVisualizer::stopThreadAction() {
    m_running = false;
    m_paused = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void DSVisualizer::waitCheck() {
    if (!m_running) {
        throw std::runtime_error("Interrupted");
    }
    while (m_paused && m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if (m_delayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_delayMs));
    }
}

void DSVisualizer::handleEvent(const sf::Event&) {}

void DSVisualizer::update(float dt) {
    updateAnimations(dt);
}

void DSVisualizer::updateAnimations(float dt) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    
    // Smooth transition animations for linear positions
    float speed = 8.f;
    for (auto& node : m_linearData) {
        node.position += (node.targetPosition - node.position) * speed * dt;
    }
}

// ==========================================
// RENDERERS
// ==========================================

void DSVisualizer::render(sf::RenderWindow& window) {
    switch (m_currentDS) {
        case DSType::Stack: renderStack(window); break;
        case DSType::Queue: renderQueue(window); break;
        case DSType::LinkedList: renderLinkedList(window); break;
        case DSType::BST: renderBST(window); break;
        case DSType::Heap: renderHeap(window); break;
    }
}

void DSVisualizer::renderStack(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    float xCenter = 700.f;
    float yBase = 580.f;
    float boxWidth = 140.f;
    float boxHeight = 40.f;

    // Draw stack container boundaries
    sf::RectangleShape lineL(sf::Vector2f(2.f, 350.f));
    lineL.setPosition({xCenter - boxWidth/2.f - 2.f, yBase - 350.f});
    lineL.setFillColor(theme.border);
    window.draw(lineL);

    sf::RectangleShape lineR(sf::Vector2f(2.f, 350.f));
    lineR.setPosition({xCenter + boxWidth/2.f, yBase - 350.f});
    lineR.setFillColor(theme.border);
    window.draw(lineR);

    sf::RectangleShape lineB(sf::Vector2f(boxWidth + 4.f, 2.f));
    lineB.setPosition({xCenter - boxWidth/2.f - 2.f, yBase});
    lineB.setFillColor(theme.border);
    window.draw(lineB);

    // Render stack frames
    for (size_t i = 0; i < m_linearData.size(); ++i) {
        float targetY = yBase - (i + 1) * (boxHeight + 2.f);
        m_linearData[i].targetPosition = {xCenter - boxWidth/2.f, targetY};
        if (m_linearData[i].position.x == 0.f) {
            m_linearData[i].position = {xCenter - boxWidth/2.f, 50.f}; // Slide down animation start
        }

        sf::RectangleShape frame(sf::Vector2f(boxWidth, boxHeight));
        frame.setPosition(m_linearData[i].position);
        frame.setFillColor(m_linearData[i].color);
        frame.setOutlineThickness(1.f);
        frame.setOutlineColor(theme.border);
        window.draw(frame);

        sf::Text valText(m_font, std::to_string(m_linearData[i].value), 14);
        valText.setFillColor(theme.text);
        sf::FloatRect bounds = valText.getLocalBounds();
        valText.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
        valText.setPosition(m_linearData[i].position + sf::Vector2f(boxWidth/2.f, boxHeight/2.f));
        window.draw(valText);
    }
}

void DSVisualizer::renderQueue(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    float xBase = 300.f;
    float yCenter = 300.f;
    float boxWidth = 60.f;
    float boxHeight = 70.f;

    // Draw pipe bounds
    sf::RectangleShape pipeTop(sf::Vector2f(600.f, 2.f));
    pipeTop.setPosition({xBase, yCenter - boxHeight/2.f - 2.f});
    pipeTop.setFillColor(theme.border);
    window.draw(pipeTop);

    sf::RectangleShape pipeBottom(sf::Vector2f(600.f, 2.f));
    pipeBottom.setPosition({xBase, yCenter + boxHeight/2.f});
    pipeBottom.setFillColor(theme.border);
    window.draw(pipeBottom);

    // Labels for Front and Rear
    sf::Text frontLabel(m_font, "◄ Front (Dequeue)", 12);
    frontLabel.setFillColor(theme.success);
    frontLabel.setPosition({xBase - 120.f, yCenter - 8.f});
    window.draw(frontLabel);

    sf::Text rearLabel(m_font, "Rear (Enqueue) ◄", 12);
    rearLabel.setFillColor(theme.primary);
    rearLabel.setPosition({xBase + 610.f, yCenter - 8.f});
    window.draw(rearLabel);

    for (size_t i = 0; i < m_linearData.size(); ++i) {
        float targetX = xBase + i * (boxWidth + 5.f);
        m_linearData[i].targetPosition = {targetX, yCenter - boxHeight/2.f};
        if (m_linearData[i].position.x == 0.f) {
            m_linearData[i].position = {xBase + 600.f, yCenter - boxHeight/2.f}; // Slide from right
        }

        sf::RectangleShape frame(sf::Vector2f(boxWidth, boxHeight));
        frame.setPosition(m_linearData[i].position);
        frame.setFillColor(m_linearData[i].color);
        frame.setOutlineThickness(1.5f);
        frame.setOutlineColor(theme.border);
        window.draw(frame);

        sf::Text valText(m_font, std::to_string(m_linearData[i].value), 14);
        valText.setFillColor(theme.text);
        sf::FloatRect bounds = valText.getLocalBounds();
        valText.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
        valText.setPosition(m_linearData[i].position + sf::Vector2f(boxWidth/2.f, boxHeight/2.f));
        window.draw(valText);
    }
}

void DSVisualizer::renderLinkedList(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    float xBase = 300.f;
    float yCenter = 300.f;
    float radius = 22.f;
    float gap = 90.f;

    for (size_t i = 0; i < m_linearData.size(); ++i) {
        float targetX = xBase + i * gap;
        m_linearData[i].targetPosition = {targetX, yCenter};
        if (m_linearData[i].position.y == 0.f) {
            m_linearData[i].position = {targetX, 100.f}; // drop from top
        }

        // Draw node pointer arrow
        if (i < m_linearData.size() - 1) {
            sf::Vertex arrow[] = {
                sf::Vertex{m_linearData[i].position + sf::Vector2f(radius, 0.f), theme.border},
                sf::Vertex{m_linearData[i+1].position - sf::Vector2f(radius, 0.f), theme.border}
            };
            window.draw(arrow, 2, sf::PrimitiveType::Lines);
            
            // Tiny arrow head
            sf::CircleShape arrowHead(4.f, 3);
            arrowHead.setOrigin({2.f, 2.f});
            arrowHead.setPosition(m_linearData[i+1].position - sf::Vector2f(radius + 2.f, 0.f));
            arrowHead.setRotation(sf::degrees(90.f));
            arrowHead.setFillColor(theme.border);
            window.draw(arrowHead);
        }

        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(m_linearData[i].position);
        circle.setFillColor(m_linearData[i].color);
        circle.setOutlineThickness(1.5f);
        circle.setOutlineColor(theme.border);
        window.draw(circle);

        sf::Text valText(m_font, std::to_string(m_linearData[i].value), 14);
        valText.setFillColor(theme.text);
        sf::FloatRect bounds = valText.getLocalBounds();
        valText.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
        valText.setPosition(m_linearData[i].position);
        window.draw(valText);

        // Pointer label next to index
        sf::Text idxText(m_font, "[" + std::to_string(i) + "]", 11);
        idxText.setFillColor(theme.text);
        idxText.setPosition(m_linearData[i].position - sf::Vector2f(10.f, radius + 18.f));
        window.draw(idxText);
    }
}

void DSVisualizer::recalculateBSTPositions(std::shared_ptr<BSTNode> node, float x, float y, float xOffset) {
    if (!node) return;
    node->position = {x, y};
    recalculateBSTPositions(node->left, x - xOffset, y + 60.f, xOffset * 0.5f);
    recalculateBSTPositions(node->right, x + xOffset, y + 60.f, xOffset * 0.5f);
}

void DSVisualizer::renderBST(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    
    // Reposition dynamically
    recalculateBSTPositions(m_bstRoot, 700.f, 100.f, 180.f);
    
    renderBSTHelper(window, m_bstRoot, {0.f, 0.f});
}

void DSVisualizer::renderBSTHelper(sf::RenderWindow& window, std::shared_ptr<BSTNode> node, sf::Vector2f parentPos) {
    if (!node) return;
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    float radius = 20.f;

    // Draw connection lines to parent
    if (parentPos.x != 0.f) {
        sf::Vertex line[] = {
            sf::Vertex{parentPos, theme.border},
            sf::Vertex{node->position, theme.border}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    renderBSTHelper(window, node->left, node->position);
    renderBSTHelper(window, node->right, node->position);

    // Draw Node circle
    sf::CircleShape circle(radius);
    circle.setOrigin({radius, radius});
    circle.setPosition(node->position);
    circle.setFillColor(node->color);
    circle.setOutlineThickness(1.5f);
    circle.setOutlineColor(theme.border);
    window.draw(circle);

    // Label
    sf::Text text(m_font, std::to_string(node->value), 13);
    text.setFillColor(theme.text);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
    text.setPosition(node->position);
    window.draw(text);
}

void DSVisualizer::renderHeap(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    if (m_heapData.empty()) return;

    // We render the heap BOTH as an array list at the bottom and a binary tree at the top!
    float radius = 20.f;
    float startX = 700.f;
    float startY = 100.f;
    float xDiff = 160.f;

    // 1. Calculate Tree Node positions
    std::vector<sf::Vector2f> positions(m_heapData.size());
    positions[0] = {startX, startY};

    for (size_t i = 0; i < m_heapData.size(); ++i) {
        int leftIdx = 2 * i + 1;
        int rightIdx = 2 * i + 2;
        int depth = static_cast<int>(std::log2(i + 1));
        float offset = xDiff / std::pow(2, depth);

        if (leftIdx < static_cast<int>(m_heapData.size())) {
            positions[leftIdx] = {positions[i].x - offset, positions[i].y + 60.f};
            
            // Draw Line
            sf::Vertex line[] = {
                sf::Vertex{positions[i], theme.border},
                sf::Vertex{positions[leftIdx], theme.border}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        if (rightIdx < static_cast<int>(m_heapData.size())) {
            positions[rightIdx] = {positions[i].x + offset, positions[i].y + 60.f};
            
            // Draw Line
            sf::Vertex line[] = {
                sf::Vertex{positions[i], theme.border},
                sf::Vertex{positions[rightIdx], theme.border}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

    // Draw Tree circles
    for (size_t i = 0; i < m_heapData.size(); ++i) {
        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(positions[i]);
        circle.setFillColor(m_heapData[i].color);
        circle.setOutlineThickness(1.5f);
        circle.setOutlineColor(theme.border);
        window.draw(circle);

        sf::Text text(m_font, std::to_string(m_heapData[i].value), 13);
        text.setFillColor(theme.text);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 3.f});
        text.setPosition(positions[i]);
        window.draw(text);
    }

    // 2. Draw Array mapping at bottom
    float arrX = 350.f;
    float arrY = 480.f;
    float blockWidth = 50.f;
    float blockHeight = 35.f;

    for (size_t i = 0; i < m_heapData.size(); ++i) {
        sf::RectangleShape block(sf::Vector2f(blockWidth - 2.f, blockHeight));
        block.setPosition({arrX + i * blockWidth, arrY});
        block.setFillColor(m_heapData[i].color);
        block.setOutlineThickness(1.f);
        block.setOutlineColor(theme.border);
        window.draw(block);

        sf::Text arrText(m_font, std::to_string(m_heapData[i].value), 12);
        arrText.setFillColor(theme.text);
        sf::FloatRect bounds = arrText.getLocalBounds();
        arrText.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f + 2.f});
        arrText.setPosition({arrX + i * blockWidth + blockWidth/2.f, arrY + blockHeight/2.f});
        window.draw(arrText);

        sf::Text idxText(m_font, std::to_string(i), 10);
        idxText.setFillColor(theme.text);
        idxText.setPosition({arrX + i * blockWidth + blockWidth/2.f - 4.f, arrY - 14.f});
        window.draw(idxText);
    }
}

// ==========================================
// OPERATIONS IMPLEMENTATION
// ==========================================

void DSVisualizer::executePush(int val) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.size() >= 8) {
        m_operationStatus = "Stack Overflow! Limit is 8 elements.";
        return;
    }
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    DSNode node;
    node.value = val;
    node.color = theme.primary;
    node.position = {0.f, 0.f}; // Anim trigger
    node.targetPosition = {0.f, 0.f};
    m_linearData.push_back(node);
    m_operationStatus = "Pushed " + std::to_string(val);
}

void DSVisualizer::executePop() {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.empty()) {
        m_operationStatus = "Stack Underflow! Empty Stack.";
        return;
    }
    int popped = m_linearData.back().value;
    m_linearData.pop_back();
    m_operationStatus = "Popped " + std::to_string(popped);
}

void DSVisualizer::executeEnqueue(int val) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.size() >= 9) {
        m_operationStatus = "Queue Full! Limit is 9 elements.";
        return;
    }
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    DSNode node;
    node.value = val;
    node.color = theme.primary;
    node.position = {0.f, 0.f};
    node.targetPosition = {0.f, 0.f};
    m_linearData.push_back(node);
    m_operationStatus = "Enqueued " + std::to_string(val);
}

void DSVisualizer::executeDequeue() {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.empty()) {
        m_operationStatus = "Queue Empty! Underflow.";
        return;
    }
    int dequeued = m_linearData.front().value;
    m_linearData.erase(m_linearData.begin());
    m_operationStatus = "Dequeued " + std::to_string(dequeued);
}

void DSVisualizer::executeListInsert(int val, int index) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.size() >= 8) {
        m_operationStatus = "List Full! Max 8 nodes.";
        return;
    }
    if (index < 0 || index > static_cast<int>(m_linearData.size())) {
        m_operationStatus = "Index out of bounds!";
        return;
    }

    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    DSNode node;
    node.value = val;
    node.color = theme.primary;
    node.position = {0.f, 0.f}; // trigger anim drop
    node.targetPosition = {0.f, 0.f};

    m_linearData.insert(m_linearData.begin() + index, node);
    m_operationStatus = "Inserted " + std::to_string(val) + " at Index " + std::to_string(index);
}

void DSVisualizer::executeListDelete(int index) {
    std::lock_guard<std::mutex> lock(m_dsMutex);
    if (m_linearData.empty()) return;
    if (index < 0 || index >= static_cast<int>(m_linearData.size())) {
        m_operationStatus = "Index out of bounds!";
        return;
    }

    int removed = m_linearData[index].value;
    m_linearData.erase(m_linearData.begin() + index);
    m_operationStatus = "Deleted " + std::to_string(removed) + " from Index " + std::to_string(index);
}

void DSVisualizer::executeListSearch(int val) {
    stopThreadAction();
    startThreadAction();

    m_workerThread = std::thread([this, val]() {
        try {
            const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
            int foundIdx = -1;

            for (size_t i = 0; i < m_linearData.size(); ++i) {
                // Flash search pointer in Red
                {
                    std::lock_guard<std::mutex> lock(m_dsMutex);
                    m_linearData[i].color = theme.danger;
                    m_operationStatus = "Checking index " + std::to_string(i);
                }
                waitCheck();

                if (m_linearData[i].value == val) {
                    foundIdx = i;
                    std::lock_guard<std::mutex> lock(m_dsMutex);
                    m_linearData[i].color = theme.success; // Found (Green)
                    m_operationStatus = "Found " + std::to_string(val) + " at Index " + std::to_string(i);
                    break;
                }

                {
                    std::lock_guard<std::mutex> lock(m_dsMutex);
                    m_linearData[i].color = theme.normal;
                }
            }

            if (foundIdx == -1) {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                m_operationStatus = std::to_string(val) + " not found in List.";
            }
        } catch(...) {}
        m_running = false;
    });
}

void DSVisualizer::executeBSTInsert(int val) {
    auto insertHelper = [](auto& self, std::shared_ptr<BSTNode>& node, int value, const Theme& theme) -> void {
        if (!node) {
            node = std::make_shared<BSTNode>();
            node->value = value;
            node->color = theme.primary;
            return;
        }
        if (value < node->value) {
            self(self, node->left, value, theme);
        } else if (value > node->value) {
            self(self, node->right, value, theme);
        }
    };

    std::lock_guard<std::mutex> lock(m_dsMutex);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    insertHelper(insertHelper, m_bstRoot, val, theme);
    m_operationStatus = "Inserted " + std::to_string(val) + " into BST";
}

void DSVisualizer::executeBSTDelete(int val) {
    auto minValNode = [](std::shared_ptr<BSTNode> node) -> std::shared_ptr<BSTNode> {
        std::shared_ptr<BSTNode> current = node;
        while (current && current->left) current = current->left;
        return current;
    };

    auto deleteHelper = [&minValNode](auto& self, std::shared_ptr<BSTNode> node, int value) -> std::shared_ptr<BSTNode> {
        if (!node) return nullptr;

        if (value < node->value) {
            node->left = self(self, node->left, value);
        } else if (value > node->value) {
            node->right = self(self, node->right, value);
        } else {
            if (!node->left) {
                return node->right;
            } else if (!node->right) {
                return node->left;
            }

            std::shared_ptr<BSTNode> temp = minValNode(node->right);
            node->value = temp->value;
            node->right = self(self, node->right, temp->value);
        }
        return node;
    };

    std::lock_guard<std::mutex> lock(m_dsMutex);
    m_bstRoot = deleteHelper(deleteHelper, m_bstRoot, val);
    m_operationStatus = "Deleted " + std::to_string(val) + " from BST";
}

void DSVisualizer::executeBSTTraversal(int type) {
    stopThreadAction();
    startThreadAction();

    m_workerThread = std::thread([this, type]() {
        try {
            std::vector<std::shared_ptr<BSTNode>> traverseOrder;
            
            // Build traversal list recursively
            auto inorder = [](auto& self, std::shared_ptr<BSTNode> node, auto& list) -> void {
                if (!node) return;
                self(self, node->left, list);
                list.push_back(node);
                self(self, node->right, list);
            };
            auto preorder = [](auto& self, std::shared_ptr<BSTNode> node, auto& list) -> void {
                if (!node) return;
                list.push_back(node);
                self(self, node->left, list);
                self(self, node->right, list);
            };
            auto postorder = [](auto& self, std::shared_ptr<BSTNode> node, auto& list) -> void {
                if (!node) return;
                self(self, node->left, list);
                self(self, node->right, list);
                list.push_back(node);
            };

            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                if (type == 0) inorder(inorder, m_bstRoot, traverseOrder);
                else if (type == 1) preorder(preorder, m_bstRoot, traverseOrder);
                else postorder(postorder, m_bstRoot, traverseOrder);
            }

            // Animate traversal sequence
            const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
            std::string pathText = "Path: ";

            for (auto node : traverseOrder) {
                {
                    std::lock_guard<std::mutex> lock(m_dsMutex);
                    node->color = theme.success;
                    pathText += std::to_string(node->value) + " ";
                    m_operationStatus = pathText;
                }
                waitCheck();
            }

            waitCheck();
            // Reset colors back
            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                auto resetColors = [&theme](auto& self, std::shared_ptr<BSTNode> n) -> void {
                    if (!n) return;
                    n->color = theme.primary;
                    self(self, n->left);
                    self(self, n->right);
                };
                resetColors(resetColors, m_bstRoot);
            }
        } catch(...) {}
        m_running = false;
    });
}

void DSVisualizer::executeHeapInsert(int val) {
    stopThreadAction();
    startThreadAction();

    m_workerThread = std::thread([this, val]() {
        try {
            const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
            
            DSNode node;
            node.value = val;
            node.color = theme.primary;
            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                m_heapData.push_back(node);
                m_operationStatus = "Inserted " + std::to_string(val) + " at end. Heapifying up...";
            }

            int curr = m_heapData.size() - 1;
            waitCheck();

            // Heapify Up
            while (curr > 0) {
                int parent = (curr - 1) / 2;
                
                {
                    std::lock_guard<std::mutex> lock(m_dsMutex);
                    m_heapData[curr].color = theme.danger;
                    m_heapData[parent].color = theme.danger;
                }
                waitCheck();

                if (m_heapData[curr].value > m_heapData[parent].value) {
                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        std::swap(m_heapData[curr], m_heapData[parent]);
                    }
                    waitCheck();
                    
                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        m_heapData[curr].color = theme.normal;
                        m_heapData[parent].color = theme.normal;
                    }
                    curr = parent;
                } else {
                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        m_heapData[curr].color = theme.normal;
                        m_heapData[parent].color = theme.normal;
                    }
                    break;
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                for(auto& n : m_heapData) n.color = theme.normal;
                m_operationStatus = "Heapify complete.";
            }
        } catch(...) {}
        m_running = false;
    });
}

void DSVisualizer::executeHeapDelete() {
    stopThreadAction();
    startThreadAction();

    m_workerThread = std::thread([this]() {
        try {
            if (m_heapData.empty()) return;
            const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

            int rootVal = m_heapData[0].value;
            
            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                m_operationStatus = "Removing Max (" + std::to_string(rootVal) + "). Swapping root with last element...";
                std::swap(m_heapData[0], m_heapData.back());
                m_heapData.pop_back();
            }

            if (m_heapData.empty()) {
                m_operationStatus = "Heap Empty.";
                return;
            }

            waitCheck();

            int curr = 0;
            int n = m_heapData.size();

            // Heapify Down
            while (true) {
                int left = 2 * curr + 1;
                int right = 2 * curr + 2;
                int largest = curr;

                if (left < n && m_heapData[left].value > m_heapData[largest].value) {
                    largest = left;
                }
                if (right < n && m_heapData[right].value > m_heapData[largest].value) {
                    largest = right;
                }

                if (largest != curr) {
                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        m_heapData[curr].color = theme.danger;
                        m_heapData[largest].color = theme.danger;
                    }
                    waitCheck();

                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        std::swap(m_heapData[curr], m_heapData[largest]);
                    }
                    waitCheck();

                    {
                        std::lock_guard<std::mutex> lock(m_dsMutex);
                        m_heapData[curr].color = theme.normal;
                        m_heapData[largest].color = theme.normal;
                    }
                    curr = largest;
                } else {
                    break;
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_dsMutex);
                for(auto& n : m_heapData) n.color = theme.normal;
                m_operationStatus = "Extract max complete. Root is now " + std::to_string(m_heapData[0].value);
            }
        } catch(...) {}
        m_running = false;
    });
}

// ==========================================
// DATA STRUCTURE IMGUI CONTROLS
// ==========================================

void DSVisualizer::renderUI() {
    ImGui::Columns(2, "DSLayout", false);
    ImGui::SetColumnWidth(0, 480);

    ImGui::Text("Structure Type:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    int currentDSIndex = static_cast<int>(m_currentDS);
    const char* dsNames[] = { "Stack", "Queue", "Linked List", "Binary Search Tree", "Max Heap" };
    if (ImGui::Combo("##DSCombo", &currentDSIndex, dsNames, IM_ARRAYSIZE(dsNames))) {
        m_currentDS = static_cast<DSType>(currentDSIndex);
        reset();
    }

    ImGui::Spacing();

    // Inputs Row
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Value", &m_inputValue);
    
    if (m_currentDS == DSType::LinkedList) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("Index", &m_inputIndex);
    }

    ImGui::Spacing();

    // Context Controls depending on active Structure
    if (m_currentDS == DSType::Stack) {
        if (ImGui::Button("Push", ImVec2(80, 25))) executePush(m_inputValue);
        ImGui::SameLine();
        if (ImGui::Button("Pop", ImVec2(80, 25))) executePop();
    } 
    else if (m_currentDS == DSType::Queue) {
        if (ImGui::Button("Enqueue", ImVec2(80, 25))) executeEnqueue(m_inputValue);
        ImGui::SameLine();
        if (ImGui::Button("Dequeue", ImVec2(80, 25))) executeDequeue();
    } 
    else if (m_currentDS == DSType::LinkedList) {
        if (ImGui::Button("Insert At", ImVec2(90, 25))) executeListInsert(m_inputValue, m_inputIndex);
        ImGui::SameLine();
        if (ImGui::Button("Delete At", ImVec2(90, 25))) executeListDelete(m_inputIndex);
        ImGui::SameLine();
        if (ImGui::Button("Search Val", ImVec2(90, 25))) executeListSearch(m_inputValue);
    } 
    else if (m_currentDS == DSType::BST) {
        if (ImGui::Button("Insert", ImVec2(80, 25))) executeBSTInsert(m_inputValue);
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(80, 25))) executeBSTDelete(m_inputValue);
        
        ImGui::Spacing();
        if (ImGui::Button("Inorder", ImVec2(80, 25))) executeBSTTraversal(0);
        ImGui::SameLine();
        if (ImGui::Button("Preorder", ImVec2(80, 25))) executeBSTTraversal(1);
        ImGui::SameLine();
        if (ImGui::Button("Postorder", ImVec2(80, 25))) executeBSTTraversal(2);
    } 
    else if (m_currentDS == DSType::Heap) {
        if (ImGui::Button("Insert Key", ImVec2(90, 25))) executeHeapInsert(m_inputValue);
        ImGui::SameLine();
        if (ImGui::Button("Extract Max", ImVec2(95, 25))) executeHeapDelete();
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear All", ImVec2(80, 25))) reset();

    ImGui::NextColumn();

    // Right Column: Status Log and Learning explanation
    ImGui::Text("OPERATION STATUS");
    ImGui::Separator();
    
    ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().primary), "%s", m_operationStatus.c_str());
    
    ImGui::Spacing();
    ImGui::Text("Concept Details:");
    ImGui::Separator();

    if (m_currentDS == DSType::Stack) {
        ImGui::TextWrapped("Stack: LIFO (Last In First Out) structure. Elements are pushed onto the top and popped off the top. Time: O(1) for push/pop.");
    } else if (m_currentDS == DSType::Queue) {
        ImGui::TextWrapped("Queue: FIFO (First In First Out) structure. Elements enter at Rear and exit from Front. Time: O(1) for enqueue/dequeue.");
    } else if (m_currentDS == DSType::LinkedList) {
        ImGui::TextWrapped("Linked List: Sequential node objects linked by pointers. Searching/deleting takes O(N) linear time.");
    } else if (m_currentDS == DSType::BST) {
        ImGui::TextWrapped("BST: Binary tree where left children < node < right children. Average search/insert is O(log N), worst is O(N) (skewed).");
    } else if (m_currentDS == DSType::Heap) {
        ImGui::TextWrapped("Max Heap: Complete binary tree where parent >= children. Useful for Priority Queues. Extract Max/Insert takes O(log N) heapify.");
    }
}
