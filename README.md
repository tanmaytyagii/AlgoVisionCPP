
<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/SFML-3.0.2-green.svg?style=for-the-badge&logo=sfml&logoColor=white" alt="SFML" />
  <img src="https://img.shields.io/badge/Dear_ImGui-1.91.5-purple.svg?style=for-the-badge" alt="Dear ImGui" />
  <img src="https://img.shields.io/badge/CMake-3.22%2B-red.svg?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
</p>

<h1 align="center">🚀 AlgoVisionCPP</h1>

<p align="center">
  <strong>Visualize • Learn • Master Algorithms</strong>
</p>

<p align="center">
  A modern C++ Data Structures & Algorithms Visualizer built with SFML, Dear ImGui, and CMake.
  Explore sorting, searching, graph traversal, pathfinding, and data structures through
  beautiful real-time interactive visualizations.
</p>

<p align="center">
  <a href="https://github.com/tanmaytyagii/AlgoVisionCPP/stargazers">
    <img src="https://img.shields.io/github/stars/tanmaytyagii/AlgoVisionCPP?style=for-the-badge&logo=github" alt="Stars" />
  </a>
  <a href="https://github.com/tanmaytyagii/AlgoVisionCPP/network/members">
    <img src="https://img.shields.io/github/forks/tanmaytyagii/AlgoVisionCPP?style=for-the-badge&logo=github" alt="Forks" />
  </a>
  <a href="https://github.com/tanmaytyagii/AlgoVisionCPP/issues">
    <img src="https://img.shields.io/github/issues/tanmaytyagii/AlgoVisionCPP?style=for-the-badge" alt="Issues" />
  </a>
  <a href="https://github.com/tanmaytyagii/AlgoVisionCPP/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/tanmaytyagii/AlgoVisionCPP?style=for-the-badge" alt="License" />
  </a>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#roadmap">Roadmap</a> •
  <a href="#contributing">Contributing</a>
</p>

---


## ⭐ Why AlgoVisionCPP?

- 🎯 Interactive Algorithm Learning
- 📊 Real-Time Performance Analytics
- ⚡ Side-by-Side Algorithm Comparisons
- 🌐 Graph & Pathfinding Visualizations
- 🎨 Modern Dark Theme UI
- 🧠 Educational Complexity Analysis
- 🏗️ Built with Modern C++17
- 🔓 Open Source & Contributor Friendly

---


## ✨ Features

Our visualizer features distinct modules designed to break down DSA processes step-by-step:

### 📊 Sorting Algorithms
*   **Bubble Sort**: Animate neighboring swaps and bubble movements.
*   **Selection Sort**: Highlight active search minimums and placement.
*   **Insertion Sort**: Show elements shifting down to clear room for the sorted prefix.
*   **Merge Sort**: Visualize recursive divides and combined merges.
*   **Quick Sort**: Animate partitioning, pivots, and dual-recursive calls.
*   **Heap Sort**: Track tree-structure binary max-heaps and element extractions.
*   **Counting Sort**: Display bucket frequencies mapping without element comparisons.
*   **Radix Sort**: Watch sorting happen digit-by-digit from least to most significant.

### 🔍 Searching Algorithms
*   **Linear Search**: Scans the array sequentially, checking each index.
*   **Binary Search**: Repeatedly halves the search interval (automatically sorts elements beforehand).
*   **Jump Search**: Jumps forward by fixed blocks ($\sqrt{N}$) and executes linear scans.

### 🕸️ Graph Algorithms
*   **BFS / DFS Traversal**: Animate queue/stack exploration frontiers.
*   **Dijkstra's Algorithm**: Visualize shortest path relaxations.
*   **Kruskal's MST**: Watch edges get sorted and added using Disjoint Set Union (DSU) checks.
*   **Prim's MST**: Grow the minimum spanning tree from a root node using priority weights.

### 🗺️ Pathfinding (Grid-Based)
*   **A\* Search**: Evaluates $f(n) = g(n) + h(n)$ using Manhattan heuristics.
*   **Dijkstra Pathfinder**: Checks uniform costs outwards from start to end.
*   **BFS Pathfinder**: Explores grid structures uniformly layer-by-layer.

### 📦 Data Structures
*   **Stack**: Push, Pop, and Peek frames on a LIFO container.
*   **Queue**: Sliding horizontal pipelines showing FIFO insertions/deletions.
*   **Linked List**: Sequentially linked circular nodes with pointer arrows.
*   **BST (Binary Search Tree)**: Dynamic branching layouts showing in-order, pre-order, and post-order paths.
*   **Max Heap**: Double layout mapping showing parent-child swaps during heapify operations.

### ⏱️ Analytics Dashboard
*   **Execution Time**: Real-time timer showing algorithm runtime coefficients.
*   **Comparisons & Swaps**: Automated counters tracking memory and comparison operations.
*   **Space & Time Complexity**: Instant complexity charts mapped to the active algorithm.


---

## 📂 Architecture & Project Structure

The project separates concerns using a decoupled MVC state design. Visualization algorithms run on a worker thread to allow speed controls (sleep delay) and pause/resume states without freezing the SFML main window.

```text
AlgoVisionCPP/
├── .github/                 # GitHub actions CI configuration and templates
├── assets/                  # Application assets (fonts, themes)
├── src/
│   ├── algorithms/          # Sorting and Searching visualization algorithms
│   ├── data_structures/     # Stack, Queue, List, BST, Heap drawers
│   ├── graph/               # Draggable graph node and traversal logic
│   ├── pathfinding/         # Grid cell structure and pathfinders (A*, Dijkstra)
│   ├── ui/                  # Topbar, sidebar, layout panels, and ThemeManager
│   ├── utils/               # ThreadSafeQueue helper classes
│   ├── VisualizerModule.hpp # Abstract module base class
│   ├── App.cpp / App.hpp    # Main frame-loop and ImGui/SFML engines
│   └── main.cpp             # Executable entry point
├── tests/                   # Target test verification suites
├── CMakeLists.txt           # Build project config
├── README.md                # Project documentation
└── LICENSE                  # MIT License
```

---

## 🛠️ Tech Stack

*   **C++17 Language Standard**: Uses structured bindings, `std::variant`, and modern thread synchronization primitives.
*   **SFML 3.0.2 (Simple and Fast Multimedia Library)**: Coordinates hardware-accelerated 2D shape rendering, coordinate matrices, and window events.
*   **Dear ImGui v1.91.5 (docking)**: Draws responsive, dockable panels, control sliders, dropdown combos, and analytics metrics.
*   **CMake**: Multi-platform build generator that manages compiler flags and project assemblies.
*   **GitHub Actions**: Continuous integration suite validating builds automatically on Windows, Linux, and macOS.

---

## 📥 Installation Guide

### Prerequisites
*   A C++17 compliant compiler (`Apple Clang`, `g++`, or `MSVC`).
*   **CMake** version 3.22 or higher.
*   **SFML 3.x** library.

### 🍎 macOS (Homebrew)
```bash
# 1. Install dependencies
brew install cmake sfml

# 2. Clone the repository
git clone https://github.com/tanmaytyagi/AlgoVisionCPP.git
cd AlgoVisionCPP

# 3. Configure and compile
cmake -B build -S .
cmake --build build --config Release

# 4. Run the visualizer
./build/AlgoVisionCPP
```

### 🐧 Linux (Ubuntu/Debian)
```bash
# 1. Install system dependencies
sudo apt-get update
sudo apt-get install -y cmake build-essential libsfml-dev libgl1-mesa-dev

# 2. Clone the repository
git clone https://github.com/tanmaytyagi/AlgoVisionCPP.git
cd AlgoVisionCPP

# 3. Configure and compile
cmake -B build -S .
cmake --build build --config Release

# 4. Run the visualizer
./build/AlgoVisionCPP
```

### 🪟 Windows (vcpkg + MSBuild)
We recommend using **vcpkg** for installing SFML on Windows:
```cmd
:: 1. Install SFML via vcpkg
vcpkg install sfml:x64-windows

:: 2. Clone the repository
git clone https://github.com/tanmaytyagi/AlgoVisionCPP.git
cd AlgoVisionCPP

:: 3. Configure CMake pointing to vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

:: 4. Build the executable
cmake --build build --config Release

:: 5. Launch
cd build/Release
AlgoVisionCPP.exe
```

---

## 🎮 Usage Guide

1.  **Tab Navigation**: Select the module from the left **Sidebar** menu (Sorting, Searching, Graph, etc.).
2.  **Algorithm Setup**: Use the bottom pane dropdowns to swap between sorting or traversal algorithms.
3.  **Simulation Speed**: Drag the **Delay (ms)** slider to speed up or slow down visual transitions.
4.  **Flow Controls**: Click **Start/Run** to begin. Utilize **Pause**, **Resume**, or **Step** to analyze step-by-step.
5.  **Interactive Canvas**:
    *   *Pathfinding*: Click and drag on empty tiles to paint wall obstacles. Drag the Green/Red circles to position start/end.
    *   *Graphs*: Select `+ Node` and click empty space to place circles. Choose `+ Edge`, click Node A, drag to Node B, and specify weight.
6.  **Theme Selection**: Toggle the theme dropdown in the top bar to switch between Dark, Light, and Cyberpunk styles.

---

## 🎓 Learning Features

This visualizer is structured to clarify abstract DSA concepts:
*   **Visual Step Alignment**: Watching Quick Sort partition elements or Dijkstra relax edges helps build mechanical algorithm intuition.
*   **Analytics Realism**: Real-world operations (comparisons and swaps count) are mapped side-by-side with theoretical space/time notation.
*   **Interactive Sandbox**: Creating custom graphs or manual array inputs helps test edge cases (e.g. sorted arrays in quicksort, cyclic graphs in Kruskal).

---

## 🏎️ Performance Comparison (Race Mode)

Navigate to **⚖ Compare Sorts** in the sidebar:
1.  Select two sorting algorithms (e.g. *Heap Sort* vs. *Quick Sort*).
2.  Adjust the shared array size slider.
3.  Click **Compare Run**. Both algorithms will sort identical datasets on separate threads.
4.  Observe the comparative analysis verdict to determine the exact speed ratio difference.

---

## 🗺️ Roadmap

- [x] Multi-threaded sorting visualization (8 algorithms)
- [x] Interactive Graph play canvas (BFS, DFS, Dijkstra, Kruskal, Prim)
- [x] Grid pathfinder with Wall editing (A*, Dijkstra, BFS)
- [x] Stack/Queue/List/BST/Heap visual representation
- [x] Dark, Light, and Cyberpunk Theme styles
- [ ] **Upcoming Features**:
  - [ ] AI-Powered runtime optimizations explainer
  - [ ] Maze generation algorithms (Recursive Division, Kruskal's maze)
  - [ ] Side-by-side Searching comparison panel
  - [ ] Exporting visualizations to GIF format
  - [ ] Red-Black Tree and AVL tree balancing rotation visualizers

---

## 🤝 Contributing

We welcome contributions from open-source developers!

1.  Fork this repository.
2.  Create a feature branch: `git checkout -b feature/amazing-new-dsa`.
3.  Ensure your code conforms to C++17 standards and passes all build checks.
4.  Commit your additions: `git commit -m "feat: add recursive maze generator"`.
5.  Submit a Pull Request using our [PR Template](.github/PULL_REQUEST_TEMPLATE.md).

### 🏷️ Open Source Tasks Available
We have marked issues suitable for first-time contributors:
*   `UI/UX`: Custom fonts integration, theme tweaks.
*   `Algorithms`: Implementing Bellman-Ford or Floyd-Warshall.
*   `Optimizations`: Shuffling/swapping animations rendering speedups.
*   `Bugs`: Fixing thread synchronization corner cases on Reset.

---

## 👥 Contributors

*   **Tanmay Tyagi** - *Creator & Maintainer* - [@tanmaytyagi](https://github.com/tanmaytyagii)
*   Feel free to submit PRs to join our contributors circle!

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

<p align="center">
  If you found this project educational or helpful, please consider giving it a ⭐!
</p>
<p align="center">
  Made with ❤️ using C++, SFML, and Dear ImGui.
</p>
