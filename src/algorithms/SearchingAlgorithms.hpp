#pragma once
#include "../VisualizerModule.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class SearchingAlgorithms : public VisualizerModule {
public:
    SearchingAlgorithms();
    ~SearchingAlgorithms() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void renderUI() override;
    void reset() override;

private:
    void generateArray();
    void startSearching();
    void stopSearching();
    void waitCheck();
    void runSearchLoop();

    // Searching algorithms
    void linearSearch();
    void binarySearch();
    void jumpSearch();

    std::vector<int> m_array;
    std::vector<sf::Color> m_colors;
    int m_arraySize;
    int m_targetValue;

    std::thread m_workerThread;
    std::mutex m_arrayMutex;
    std::atomic<bool> m_searching;
    std::atomic<bool> m_paused;
    std::atomic<bool> m_stepRequested;
    std::atomic<int> m_delayMs;

    std::atomic<int> m_comparisons;
    std::atomic<int> m_foundIndex; // -1 if not found, index if found
    float m_elapsedTime;
    sf::Clock m_timerClock;

    int m_selectedAlgo;
    std::vector<std::string> m_algoNames;
};
