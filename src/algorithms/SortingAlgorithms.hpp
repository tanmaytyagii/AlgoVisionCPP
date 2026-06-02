#pragma once
#include "../VisualizerModule.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

class SortingAlgorithms : public VisualizerModule {
public:
    SortingAlgorithms();
    ~SortingAlgorithms() override;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    void renderInBounds(sf::RenderWindow& window, float xStart, float yStart, float width, float height);
    void renderUI() override;
    void reset() override;

    // Side-by-side performance helper getters
    bool isSorting() const { return m_sorting; }
    int getComparisons() const { return m_comparisons; }
    int getSwaps() const { return m_swaps; }
    float getElapsedTime() const { return m_elapsedTime; }
    void startExternalSort(int algoIndex, int size); // Used by side-by-side comparison

private:
    void generateArray();
    void startSorting();
    void stopSorting();
    void waitCheck(); // Handles pause, speed delay, and step-by-step controls
    void runSortingLoop(); // Worker thread entry point

    // Sorting algorithm implementations (running on worker thread)
    void bubbleSort();
    void selectionSort();
    void insertionSort();
    void mergeSort(int l, int r);
    void merge(int l, int m, int r);
    void quickSort(int l, int r);
    int partition(int l, int r);
    void heapSort();
    void heapify(int n, int i);
    void countingSort();
    void radixSort();

    // Data structures for array representation
    std::vector<int> m_array;
    std::vector<sf::Color> m_colors;
    int m_arraySize;

    // Multithreading and flow control
    std::thread m_workerThread;
    std::mutex m_arrayMutex;
    std::atomic<bool> m_sorting;
    std::atomic<bool> m_paused;
    std::atomic<bool> m_stepRequested;
    std::atomic<int> m_delayMs;

    // Metrics tracking
    std::atomic<int> m_comparisons;
    std::atomic<int> m_swaps;
    float m_elapsedTime;
    sf::Clock m_timerClock;

    // Selected algorithm controls
    int m_selectedAlgo;
    std::vector<std::string> m_algoNames;
    bool m_customInputMode;
    char m_customInputBuffer[256];
};
