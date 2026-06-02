# Contributing to AlgoVisionCPP

Welcome to the AlgoVisionCPP community! We are excited to have you contribute to this project. As a developer or designer, your feedback and code additions make this DSA visualizer better for everyone.

Please review the guidelines below to ensure a smooth contribution process.

---

## Code Guidelines

To maintain high code quality and architectural integrity, we adhere to the following standards:

1. **Language Standard**: C++17 (or later). Ensure your code compiles without warnings on major compilers (`clang++`, `g++`, `MSVC`).
2. **Coding Standards**:
   - Follow SOLID principles to maintain loose coupling.
   - Use meaningful variable, class, and method names in `camelCase` for methods and variables, and `PascalCase` for classes.
   - Prefix member variables with `m_` (e.g., `m_delayMs`).
3. **Documentation**:
   - Provide Doxygen-style comments for all public class definitions and function declarations.
   - Keep READMEs and walkthroughs updated for any major feature changes.
4. **Visual Layout Formatting**:
   - Use `clang-format` to format all code. You can run the formatter in your editor or via CLI using our shared `.clang-format` configuration.

---

## Contribution Workflow

1. **Fork the Repository**: Clone your fork locally.
2. **Create a Feature Branch**: Keep it distinct and descriptive:
   ```bash
   git checkout -b feature/awesome-new-algorithm
   ```
3. **Develop and Test**: Ensure the code builds and the visualizations work smoothly across all three themes (Dark, Light, Cyberpunk).
4. **Commit Your Changes**: Follow clear commit guidelines:
   ```bash
   git commit -m "feat: implement radix sort visualization animations"
   ```
5. **Push and Submit**: Push to your fork and submit a Pull Request (PR) to our main branch. Fill out the PR template thoroughly.

---

## Reporting Issues

If you discover a bug or want to suggest an improvement, please open an Issue using the appropriate template:
- **Bug Reports**: Describe the problem, steps to reproduce, and environment specifications.
- **Feature Requests**: Outline the proposed additions, target visual layout, and educational benefit.

Thank you for contributing! Let's build the ultimate C++ algorithm visualizer together.
