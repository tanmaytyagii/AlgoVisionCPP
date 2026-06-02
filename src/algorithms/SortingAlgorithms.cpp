#include "SortingAlgorithms.hpp"
#include "../ui/ThemeManager.hpp"
#include <imgui.h>
#include <random>
#include <sstream>
#include <iostream>
#include <algorithm>

SortingAlgorithms::SortingAlgorithms()
    : m_arraySize(50)
    , m_sorting(false)
    , m_paused(false)
    , m_stepRequested(false)
    , m_delayMs(20)
    , m_comparisons(0)
    , m_swaps(0)
    , m_elapsedTime(0.f)
    , m_selectedAlgo(0)
    , m_customInputMode(false) {
    
    m_algoNames = {
        "Bubble Sort",
        "Selection Sort",
        "Insertion Sort",
        "Merge Sort",
        "Quick Sort",
        "Heap Sort",
        "Counting Sort",
        "Radix Sort"
    };
    
    m_customInputBuffer[0] = '\0';
    generateArray();
}

SortingAlgorithms::~SortingAlgorithms() {
    stopSorting();
}

void SortingAlgorithms::generateArray() {
    stopSorting();

    std::lock_guard<std::mutex> lock(m_arrayMutex);
    m_array.resize(m_arraySize);
    m_colors.resize(m_arraySize);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 350);

    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (int i = 0; i < m_arraySize; ++i) {
        m_array[i] = dis(gen);
        m_colors[i] = theme.normal;
    }

    m_comparisons = 0;
    m_swaps = 0;
    m_elapsedTime = 0.f;
}

void SortingAlgorithms::startSorting() {
    stopSorting();

    m_sorting = true;
    m_paused = false;
    m_stepRequested = false;
    m_comparisons = 0;
    m_swaps = 0;
    m_elapsedTime = 0.f;
    m_timerClock.restart();

    // Reset bar colors to normal before starting
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (auto& color : m_colors) {
        color = theme.normal;
    }

    m_workerThread = std::thread(&SortingAlgorithms::runSortingLoop, this);
}

void SortingAlgorithms::stopSorting() {
    m_sorting = false;
    m_paused = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void SortingAlgorithms::waitCheck() {
    if (!m_sorting) {
        throw std::runtime_error("Interrupted");
    }

    while (m_paused && m_sorting) {
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

void SortingAlgorithms::runSortingLoop() {
    try {
        switch (m_selectedAlgo) {
            case 0: bubbleSort(); break;
            case 1: selectionSort(); break;
            case 2: insertionSort(); break;
            case 3: mergeSort(0, m_array.size() - 1); break;
            case 4: quickSort(0, m_array.size() - 1); break;
            case 5: heapSort(); break;
            case 6: countingSort(); break;
            case 7: radixSort(); break;
        }

        // Color whole array green once finished
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
            for (size_t i = 0; i < m_array.size(); ++i) {
                m_colors[i] = theme.success;
            }
        }
        m_sorting = false;
    }
    catch (const std::exception&) {
        // Suppress exception; thread is winding down
    }
}

void SortingAlgorithms::handleEvent(const sf::Event&) {}

void SortingAlgorithms::update(float) {
    if (m_sorting && !m_paused) {
        m_elapsedTime = m_timerClock.getElapsedTime().asSeconds();
    }
}

void SortingAlgorithms::render(sf::RenderWindow& window) {
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);

    // Reserved workspace coords
    float xStart = 240.f;
    float yStart = 75.f;
    float width = windowWidth - xStart - 20.f;
    float height = windowHeight - 220.f - yStart - 20.f;

    renderInBounds(window, xStart, yStart, width, height);
}

void SortingAlgorithms::renderInBounds(sf::RenderWindow& window, float xStart, float yStart, float width, float height) {
    std::lock_guard<std::mutex> lock(m_arrayMutex);

    if (m_array.empty()) return;

    float barWidth = width / m_array.size();
    int maxVal = *std::max_element(m_array.begin(), m_array.end());
    float scaleY = height / (maxVal > 0 ? maxVal : 1);

    for (size_t i = 0; i < m_array.size(); ++i) {
        float barHeight = m_array[i] * scaleY;
        sf::RectangleShape bar(sf::Vector2f(barWidth - (m_array.size() > 100 ? 0.f : 1.f), barHeight));
        bar.setPosition({xStart + i * barWidth, yStart + height - barHeight});
        bar.setFillColor(m_colors[i]);
        window.draw(bar);
    }
}

void SortingAlgorithms::reset() {
    stopSorting();
    generateArray();
}

// ==========================================
// ALGORITHM IMPLEMENTATIONS (Threaded)
// ==========================================

void SortingAlgorithms::bubbleSort() {
    int n = m_array.size();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[j] = theme.danger;
                m_colors[j + 1] = theme.danger;
            }
            m_comparisons++;

            if (m_array[j] > m_array[j + 1]) {
                std::swap(m_array[j], m_array[j + 1]);
                m_swaps++;
                waitCheck();
            }

            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[j] = theme.normal;
                m_colors[j + 1] = theme.normal;
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[n - i - 1] = theme.success;
        }
    }
}

void SortingAlgorithms::selectionSort() {
    int n = m_array.size();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j) {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[j] = theme.danger;
                m_colors[minIdx] = theme.secondary; // yellow highlight for current min
            }
            m_comparisons++;

            if (m_array[j] < m_array[minIdx]) {
                {
                    std::lock_guard<std::mutex> lock(m_arrayMutex);
                    m_colors[minIdx] = theme.normal;
                }
                minIdx = j;
            } else {
                {
                    std::lock_guard<std::mutex> lock(m_arrayMutex);
                    m_colors[j] = theme.normal;
                }
            }
        }
        if (minIdx != i) {
            std::swap(m_array[i], m_array[minIdx]);
            m_swaps++;
        }
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[minIdx] = theme.normal;
            m_colors[i] = theme.success;
        }
    }
}

void SortingAlgorithms::insertionSort() {
    int n = m_array.size();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (int i = 1; i < n; ++i) {
        int key = m_array[i];
        int j = i - 1;

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.primary; // Blue for key
        }

        while (j >= 0 && m_array[j] > key) {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[j] = theme.danger; // Red for comparison
            }
            m_comparisons++;

            m_array[j + 1] = m_array[j];
            m_swaps++;
            
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[j] = theme.normal;
            }
            j--;
        }
        m_array[j + 1] = key;
        
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            for(int k = 0; k <= i; ++k) {
                m_colors[k] = theme.success;
            }
        }
    }
}

void SortingAlgorithms::merge(int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    std::vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = m_array[l + i];
    for (int j = 0; j < n2; j++) R[j] = m_array[m + 1 + j];

    int i = 0, j = 0, k = l;
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    while (i < n1 && j < n2) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[k] = theme.danger;
        }
        m_comparisons++;

        if (L[i] <= R[j]) {
            m_array[k] = L[i];
            i++;
        } else {
            m_array[k] = R[j];
            j++;
        }
        m_swaps++;
        
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[k] = theme.primary;
        }
        k++;
    }

    while (i < n1) {
        waitCheck();
        m_array[k] = L[i];
        m_swaps++;
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[k] = theme.primary;
        }
        i++;
        k++;
    }

    while (j < n2) {
        waitCheck();
        m_array[k] = R[j];
        m_swaps++;
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[k] = theme.primary;
        }
        j++;
        k++;
    }
}

void SortingAlgorithms::mergeSort(int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(l, m);
        mergeSort(m + 1, r);
        merge(l, m, r);
    }
}

int SortingAlgorithms::partition(int l, int r) {
    int pivot = m_array[r];
    int i = (l - 1);
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    {
        std::lock_guard<std::mutex> lock(m_arrayMutex);
        m_colors[r] = theme.secondary; // pivot is yellow
    }

    for (int j = l; j <= r - 1; j++) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[j] = theme.danger; // red for checked
        }
        m_comparisons++;

        if (m_array[j] < pivot) {
            i++;
            std::swap(m_array[i], m_array[j]);
            m_swaps++;
            waitCheck();
        }

        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[j] = theme.normal;
        }
    }
    std::swap(m_array[i + 1], m_array[r]);
    m_swaps++;
    
    {
        std::lock_guard<std::mutex> lock(m_arrayMutex);
        m_colors[r] = theme.normal;
        m_colors[i + 1] = theme.success;
    }

    return (i + 1);
}

void SortingAlgorithms::quickSort(int l, int r) {
    if (l < r) {
        int pi = partition(l, r);
        quickSort(l, pi - 1);
        quickSort(pi + 1, r);
    }
}

void SortingAlgorithms::heapify(int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    waitCheck();
    {
        std::lock_guard<std::mutex> lock(m_arrayMutex);
        m_colors[i] = theme.primary;
    }

    if (l < n) {
        m_comparisons++;
        if (m_array[l] > m_array[largest]) largest = l;
    }

    if (r < n) {
        m_comparisons++;
        if (m_array[r] > m_array[largest]) largest = r;
    }

    if (largest != i) {
        std::swap(m_array[i], m_array[largest]);
        m_swaps++;
        
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.danger;
            m_colors[largest] = theme.danger;
        }

        heapify(n, largest);
    }

    {
        std::lock_guard<std::mutex> lock(m_arrayMutex);
        m_colors[i] = theme.normal;
    }
}

void SortingAlgorithms::heapSort() {
    int n = m_array.size();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        std::swap(m_array[0], m_array[i]);
        m_swaps++;
        
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.success;
        }

        heapify(i, 0);
    }
}

void SortingAlgorithms::countingSort() {
    int n = m_array.size();
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();

    int maxVal = *std::max_element(m_array.begin(), m_array.end());
    std::vector<int> count(maxVal + 1, 0);
    std::vector<int> output(n);

    for (int i = 0; i < n; i++) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.danger;
        }
        count[m_array[i]]++;
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.normal;
        }
    }

    for (size_t i = 1; i < count.size(); i++) {
        count[i] += count[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        output[count[m_array[i]] - 1] = m_array[i];
        count[m_array[i]]--;
    }

    for (int i = 0; i < n; i++) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_array[i] = output[i];
            m_colors[i] = theme.primary;
            m_swaps++;
        }
    }
}

void SortingAlgorithms::radixSort() {
    int maxVal = *std::max_element(m_array.begin(), m_array.end());
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    int n = m_array.size();

    auto countSortForRadix = [this, n, theme](int exp) {
        std::vector<int> output(n);
        std::vector<int> count(10, 0);

        for (int i = 0; i < n; i++) {
            count[(m_array[i] / exp) % 10]++;
        }

        for (int i = 1; i < 10; i++) {
            count[i] += count[i - 1];
        }

        for (int i = n - 1; i >= 0; i--) {
            output[count[(m_array[i] / exp) % 10] - 1] = m_array[i];
            count[(m_array[i] / exp) % 10]--;
        }

        for (int i = 0; i < n; i++) {
            waitCheck();
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_array[i] = output[i];
                m_colors[i] = theme.primary;
                m_swaps++;
            }
        }
    };

    for (int exp = 1; maxVal / exp > 0; exp *= 10) {
        countSortForRadix(exp);
    }
}

// ==========================================
// IMGUI CONTROLS & LEARNING
// ==========================================

void SortingAlgorithms::renderUI() {
    // Left: control buttons and settings
    ImGui::Columns(2, "SortingLayout", false);
    ImGui::SetColumnWidth(0, 480);

    ImGui::Text("Algorithm:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    if (ImGui::BeginCombo("##AlgoCombo", m_algoNames[m_selectedAlgo].c_str())) {
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

    // Start/Pause/Step/Reset row
    if (m_sorting) {
        if (m_paused) {
            if (ImGui::Button("Resume", ImVec2(80, 25))) {
                m_paused = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Step ⏵", ImVec2(80, 25))) {
                m_stepRequested = true;
            }
        } else {
            if (ImGui::Button("Pause", ImVec2(80, 25))) {
                m_paused = true;
            }
        }
    } else {
        if (ImGui::Button("Start", ImVec2(80, 25))) {
            startSorting();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80, 25))) {
        reset();
    }

    // Array configuration row
    ImGui::Spacing();
    ImGui::SetNextItemWidth(200);
    if (ImGui::SliderInt("Array Size", &m_arraySize, 10, 300)) {
        if (!m_sorting) {
            generateArray();
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    int delayVal = m_delayMs.load();
    if (ImGui::SliderInt("Delay (ms)", &delayVal, 0, 200)) {
        m_delayMs = delayVal;
    }

    // Custom input section
    ImGui::Spacing();
    ImGui::Checkbox("Custom Manual Array", &m_customInputMode);
    if (m_customInputMode) {
        ImGui::InputText("CSV (e.g. 50,20,100,45)", m_customInputBuffer, IM_ARRAYSIZE(m_customInputBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Apply")) {
            stopSorting();
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_array.clear();
            std::stringstream ss(m_customInputBuffer);
            std::string token;
            while (std::getline(ss, token, ',')) {
                try {
                    m_array.push_back(std::stoi(token));
                } catch (...) {}
            }
            if (!m_array.empty()) {
                m_arraySize = m_array.size();
                m_colors.resize(m_arraySize);
                const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
                for (auto& color : m_colors) {
                    color = theme.normal;
                }
            } else {
                generateArray();
            }
        }
    }

    // Analytics section
    ImGui::NextColumn();
    
    ImGui::Text("ANALYTICS");
    ImGui::Separator();
    
    ImGui::Columns(3, "Metrics", false);
    ImGui::SetColumnWidth(0, 150);
    ImGui::SetColumnWidth(1, 150);
    ImGui::SetColumnWidth(2, 180);

    ImGui::Metric("Timer", "%.2fs", m_elapsedTime);
    ImGui::NextColumn();
    ImGui::Metric("Comparisons", "%d", m_comparisons.load());
    ImGui::NextColumn();
    ImGui::Metric("Swaps", "%d", m_swaps.load());
    
    ImGui::Columns(1);
    ImGui::Spacing();

    // Learning Details Panel
    ImGui::Text("Complexity Info:");
    ImGui::Separator();
    if (m_selectedAlgo == 0) { // Bubble Sort
        ImGui::Text("Bubble Sort: O(N^2) Time  |  O(1) Space. Bubble sort repeatedly swaps adjacent elements if out of order.");
    } else if (m_selectedAlgo == 1) { // Selection
        ImGui::Text("Selection Sort: O(N^2) Time  |  O(1) Space. Selects the smallest element in unsorted region and swaps to index.");
    } else if (m_selectedAlgo == 2) { // Insertion
        ImGui::Text("Insertion Sort: O(N^2) Time  |  O(1) Space. Places elements into their correct position in a sorted prefix.");
    } else if (m_selectedAlgo == 3) { // Merge
        ImGui::Text("Merge Sort: O(N log N) Time  |  O(N) Space. Divide-and-conquer algorithm that recursively splits and merges.");
    } else if (m_selectedAlgo == 4) { // Quick
        ImGui::Text("Quick Sort: O(N log N) Avg, O(N^2) Worst Time  |  O(log N) Space. Partitions around pivot recursively.");
    } else if (m_selectedAlgo == 5) { // Heap
        ImGui::Text("Heap Sort: O(N log N) Time  |  O(1) Space. Builds a binary max-heap and repeatedly extracts the maximum.");
    } else if (m_selectedAlgo == 6) { // Counting
        ImGui::Text("Counting Sort: O(N + K) Time  |  O(N + K) Space. Non-comparative sorting based on keys frequencies mapping.");
    } else if (m_selectedAlgo == 7) { // Radix
        ImGui::Text("Radix Sort: O(N * D) Time  |  O(N + K) Space. Non-comparative digit-by-digit sorting using counting sort helper.");
    }
}

void SortingAlgorithms::startExternalSort(int algoIndex, int size) {
    m_selectedAlgo = algoIndex;
    m_arraySize = size;
    generateArray();
    startSorting();
}
