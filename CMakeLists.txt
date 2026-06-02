cmake_minimum_required(VERSION 3.22)
project(AlgoVisionCPP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Find SFML 3.x
find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Window System)

# Fetch Dear ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5
)
FetchContent_MakeAvailable(imgui)

# Fetch ImGui-SFML
# Note: For SFML 3, we use imgui-sfml v3.0
FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG        v3.0
)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
# ImGui-SFML CMake configuration options
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Do not find SFML within imgui-sfml since we found it already" FORCE)
FetchContent_MakeAvailable(imgui-sfml)

# Source files list
set(SOURCES
    src/main.cpp
    src/App.cpp
    src/ui/ThemeManager.cpp
    src/algorithms/SortingAlgorithms.cpp
    src/algorithms/SearchingAlgorithms.cpp
    src/graph/GraphVisualizer.cpp
    src/pathfinding/PathfindingVisualizer.cpp
    src/data_structures/DSVisualizer.cpp
)

# Header files list (for IDE listing)
set(HEADERS
    src/App.hpp
    src/VisualizerModule.hpp
    src/ui/ThemeManager.hpp
    src/algorithms/SortingAlgorithms.hpp
    src/algorithms/SearchingAlgorithms.hpp
    src/graph/GraphVisualizer.hpp
    src/pathfinding/PathfindingVisualizer.hpp
    src/data_structures/DSVisualizer.hpp
    src/utils/ThreadSafeQueue.hpp
)

add_executable(AlgoVisionCPP ${SOURCES} ${HEADERS})

# Link SFML and ImGui-SFML
target_link_libraries(AlgoVisionCPP PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
)

# Set compile definitions for ImGui-SFML
target_compile_definitions(AlgoVisionCPP PRIVATE IMGUI_USER_CONFIG="imconfig-SFML.h")

# Copy assets to build directory
add_custom_command(TARGET AlgoVisionCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:AlgoVisionCPP>/assets
)
