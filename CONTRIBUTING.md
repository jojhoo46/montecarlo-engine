# Contributing to Montecarlo Engine

First off, thank you for considering contributing to this project! 

This tool is designed to bridge the gap between high-performance C++ computing and Python-based data workflows or web backends. Our primary goal is **speed, mathematical accuracy, and memory safety**.

## Philosophy
We want to keep this engine lightweight and brutally fast. When proposing changes, ask yourself:
1. *Does this introduce a conditional branch inside the main OpenMP loop?* (If yes, please rethink the mathematical abstraction).
2. *Does this increase memory overhead or cause cache misses?*
3. *Is the math rigorously verifiable?*

## How to Contribute

### 1. Reporting Bugs
If you find a bug or a mathematical discrepancy, please open an issue with:
* Your OS and Compiler version.
* The Python script used to trigger the bug.
* The expected mathematical output (preferably compared against Black-Scholes analytical formulas).

### 2. Suggesting Enhancements
We are open to optimizations (e.g., implementing Ziggurat instead of Box-Muller, SIMD vectorization, or adding new option types like American or Asian). Please open an issue to discuss the mathematical approach before writing the code.

### 3. Submitting Pull Requests (PRs)
1. **Fork the repository** and create your branch from `main`.
2. **Setup your environment:** Ensure you can build the project locally using CMake and that your compiler supports C++20.
3. **Write Tests:** If you add a new feature, you must add a Pytest script demonstrating its accuracy (e.g., verifying Put-Call parity or calculating specific Greeks).
4. **Code Style:** * Use modern C++ practices (`const` correctness, `std::move` where appropriate).
   * Document new C++ functions using Doxygen style (`///` or `/** */`).
5. **Commit cleanly:** Write clear, concise commit messages.
6. **Open the PR:** Describe exactly what you changed and include performance benchmarks (execution time before and after your PR) if you optimized existing code.

## Testing Guidelines
All mathematical models must be rigorously tested. We use Python (`pytest`) to validate the compiled binaries. 
Any PR that alters the pricing logic will be evaluated against fundamental financial theorems:
* The price must fall within the 95% confidence interval for large N.
* Put-Call parity must hold: `C - P = S_0 - K * exp(-rT)`
* Greeks must converge to their analytical counterparts.

Thank you for helping make open-source financial engineering faster and more accessible!