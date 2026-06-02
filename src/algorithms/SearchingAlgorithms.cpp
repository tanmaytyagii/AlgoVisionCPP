#include "SearchingAlgorithms.hpp"
#include "../ui/ThemeManager.hpp"
#include <imgui.h>
#include <random>
#include <cmath>
#include <algorithm>

SearchingAlgorithms::SearchingAlgorithms()
    : m_arraySize(30)
    , m_targetValue(0)
    , m_searching(false)
    , m_paused(false)
    , m_stepRequested(false)
    , m_delayMs(150)
    , m_comparisons(0)
    , m_foundIndex(-1)
    , m_elapsedTime(0.f)
    , m_selectedAlgo(0) {

    m_algoNames = {
        "Linear Search",
        "Binary Search",
        "Jump Search"
    };

    generateArray();
}

SearchingAlgorithms::~SearchingAlgorithms() {
    stopSearching();
}

void SearchingAlgorithms::generateArray() {
    stopSearching();
    std::lock_guard<std::mutex> lock(m_arrayMutex);
    
    m_array.resize(m_arraySize);
    m_colors.resize(m_arraySize);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 150);

    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (int i = 0; i < m_arraySize; ++i) {
        m_array[i] = dis(gen);
        m_colors[i] = theme.normal;
    }

    // Sort if Binary or Jump search are selected
    if (m_selectedAlgo == 1 || m_selectedAlgo == 2) {
        std::sort(m_array.begin(), m_array.end());
    }

    // Pick a random target from the array as default
    if (!m_array.empty()) {
        std::uniform_int_distribution<> targetDis(0, m_arraySize - 1);
        m_targetValue = m_array[targetDis(gen)];
    }

    m_comparisons = 0;
    m_foundIndex = -1;
    m_elapsedTime = 0.f;
}

void SearchingAlgorithms::startSearching() {
    stopSearching();

    m_searching = true;
    m_paused = false;
    m_stepRequested = false;
    m_comparisons = 0;
    m_foundIndex = -1;
    m_elapsedTime = 0.f;
    m_timerClock.restart();

    // Reset colors
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (auto& color : m_colors) {
        color = theme.normal;
    }

    m_workerThread = std::thread(&SearchingAlgorithms::runSearchLoop, this);
}

void SearchingAlgorithms::stopSearching() {
    m_searching = false;
    m_paused = false;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void SearchingAlgorithms::waitCheck() {
    if (!m_searching) {
        throw std::runtime_error("Interrupted");
    }

    while (m_paused && m_searching) {
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

void SearchingAlgorithms::runSearchLoop() {
    try {
        switch (m_selectedAlgo) {
            case 0: linearSearch(); break;
            case 1: binarySearch(); break;
            case 2: jumpSearch(); break;
        }
        m_searching = false;
    }
    catch (const std::exception&) {
        m_searching = false;
    }
}

void SearchingAlgorithms::handleEvent(const sf::Event&) {}

void SearchingAlgorithms::update(float) {
    if (m_searching && !m_paused) {
        m_elapsedTime = m_timerClock.getElapsedTime().asSeconds();
    }
}

void SearchingAlgorithms::render(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(m_arrayMutex);

    if (m_array.empty()) return;

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);

    float xStart = 240.f;
    float yStart = 75.f;
    float width = windowWidth - xStart - 20.f;
    float height = windowHeight - 220.f - yStart - 20.f;

    float barWidth = width / m_array.size();
    int maxVal = *std::max_element(m_array.begin(), m_array.end());
    float scaleY = height / (maxVal > 0 ? maxVal : 1);

    for (size_t i = 0; i < m_array.size(); ++i) {
        float barHeight = m_array[i] * scaleY;
        sf::RectangleShape bar(sf::Vector2f(barWidth - 2.f, barHeight));
        bar.setPosition({xStart + i * barWidth, yStart + height - barHeight});
        bar.setFillColor(m_colors[i]);
        window.draw(bar);
    }
}

void SearchingAlgorithms::reset() {
    stopSearching();
    generateArray();
}

// ==========================================
// SEARCH ALGORITHMS
// ==========================================

void SearchingAlgorithms::linearSearch() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    for (size_t i = 0; i < m_array.size(); ++i) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.danger; // Red for comparison
        }
        m_comparisons++;

        if (m_array[i] == m_targetValue) {
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[i] = theme.success; // Green for target found
                m_foundIndex = i;
            }
            return;
        }

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[i] = theme.inactive; // Gray out traversed
        }
    }
}

void SearchingAlgorithms::binarySearch() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    int low = 0;
    int high = m_array.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            // Highlight boundaries in auxiliary purple/blue, and mid in Red
            for (int k = 0; k < static_cast<int>(m_array.size()); ++k) {
                if (k >= low && k <= high) {
                    m_colors[k] = theme.normal;
                } else {
                    m_colors[k] = theme.inactive;
                }
            }
            m_colors[mid] = theme.danger;
        }
        m_comparisons++;

        if (m_array[mid] == m_targetValue) {
            {
                std::lock_guard<std::mutex> lock(m_arrayMutex);
                m_colors[mid] = theme.success;
                m_foundIndex = mid;
            }
            return;
        }

        if (m_array[mid] < m_targetValue) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
}

void SearchingAlgorithms::jumpSearch() {
    const Theme& theme = ThemeManager::getInstance().getCurrentTheme();
    int n = m_array.size();
    int step = std::sqrt(n);
    int prev = 0;

    waitCheck();
    {
        std::lock_guard<std::mutex> lock(m_arrayMutex);
        m_colors[0] = theme.primary;
    }

    // Finding the block where element is present
    while (m_array[std::min(step, n) - 1] < m_targetValue) {
        m_comparisons++;
        prev = step;
        step += std::sqrt(n);

        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[std::min(prev, n) - 1] = theme.danger;
            if (prev < n) m_colors[prev] = theme.primary;
        }

        if (prev >= n) return;
    }

    // Linear search in block
    while (m_array[prev] < m_targetValue) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[prev] = theme.danger;
        }
        m_comparisons++;
        prev++;

        if (prev == std::min(step, n)) return;
    }

    m_comparisons++;
    if (m_array[prev] == m_targetValue) {
        waitCheck();
        {
            std::lock_guard<std::mutex> lock(m_arrayMutex);
            m_colors[prev] = theme.success;
            m_foundIndex = prev;
        }
    }
}

// ==========================================
// IMGUI PANEL & DETAILS
// ==========================================

void SearchingAlgorithms::renderUI() {
    ImGui::Columns(2, "SearchingLayout", false);
    ImGui::SetColumnWidth(0, 480);

    ImGui::Text("Algorithm:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    if (ImGui::BeginCombo("##SearchCombo", m_algoNames[m_selectedAlgo].c_str())) {
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
    if (m_searching) {
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
        if (ImGui::Button("Search", ImVec2(80, 25))) {
            startSearching();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80, 25))) {
        reset();
    }

    // Input target value
    ImGui::Spacing();
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Target Value", &m_targetValue);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    int delayVal = m_delayMs.load();
    if (ImGui::SliderInt("Delay (ms)", &delayVal, 50, 1000)) {
        m_delayMs = delayVal;
    }

    ImGui::NextColumn();

    // Metrics & complexity
    ImGui::Text("ANALYTICS");
    ImGui::Separator();
    
    ImGui::Columns(3, "SearchMetrics", false);
    ImGui::SetColumnWidth(0, 150);
    ImGui::SetColumnWidth(1, 150);
    ImGui::SetColumnWidth(2, 180);

    ImGui::Metric("Timer", "%.2fs", m_elapsedTime);
    ImGui::NextColumn();
    ImGui::Metric("Comparisons", "%d", m_comparisons.load());
    ImGui::NextColumn();
    
    if (m_foundIndex != -1) {
        ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().success), "Found at index %d", m_foundIndex.load());
    } else if (!m_searching && m_comparisons > 0) {
        ImGui::TextColored(ThemeManager::toImVec4(ThemeManager::getInstance().getCurrentTheme().danger), "Target Not Found");
    } else {
        ImGui::Text("Searching...");
    }
    
    ImGui::Columns(1);
    ImGui::Spacing();
    
    ImGui::Text("Complexity Info:");
    ImGui::Separator();
    if (m_selectedAlgo == 0) {
        ImGui::Text("Linear Search: O(N) Time  |  O(1) Space. Traverses elements one-by-one from first to last.");
    } else if (m_selectedAlgo == 1) {
        ImGui::Text("Binary Search: O(log N) Time  |  O(1) Space. Repeatedly divides search interval in half. Needs sorted elements.");
    } else if (m_selectedAlgo == 2) {
        ImGui::Text("Jump Search: O(√N) Time  |  O(1) Space. Jumps ahead by fixed steps (√N) and performs linear search. Needs sorted list.");
    }
}
