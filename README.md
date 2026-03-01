# High-Performance Montecarlo Options Engine 

[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Python](https://img.shields.io/badge/Python-3.8+-yellow.svg)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

A hyper-optimized, multithreaded European Options pricing engine built in modern **C++20** and exposed to **Python** via `pybind11`. 

This project simulates stochastic price paths to calculate the Fair Value and the central finite-difference "Greeks" (Delta, Gamma, Vega) of financial derivatives in milliseconds.

## Key Features

* **Blazing Fast Concurrency:** Powered by OpenMP for true CPU core utilization.
* **Thread-Safe PRNG:** Implements the state-of-the-art `xoshiro256**` algorithm for rapid, collision-free pseudo-random number generation across multiple threads.
* **Branchless Architecture:** Mathematical implementations (like Call/Put payoff calculations) are designed without conditional branching in the hot loop to maximize CPU pipeline efficiency.
* **Python Bindings:** Seamless integration into any Python backend (Django, FastAPI) or data science workflow (Jupyter, Pandas) using `pybind11`.
* **Cross-Platform CMake:** Configured to compile gracefully using GCC/MinGW (Linux/Windows) or MSVC (Windows).

## Mathematical Foundations

The engine assumes the underlying asset follows Geometric Brownian Motion (GBM) under the risk-neutral measure:

$$dS_t = r S_t dt + \sigma S_t dW_t$$

Where the terminal price $S_T$ is simulated using the exact solution:

$$S_T = S_0 \exp\left( \left(r - \frac{\sigma^2}{2}\right)T + \sigma \sqrt{T} Z \right)$$

With $Z \sim \mathcal{N}(0,1)$ generated using the Box-Muller transform. The payoff function is branchless, relying on the directional multiplier $\phi$ (1 for Call, -1 for Put):

$$V(S_T) = \max(\phi(S_T - K), 0)$$

## Build Instructions

### Requirements
* **C++20** compatible compiler (GCC, Clang, or MSVC).
* **CMake** (3.15 or higher).
* **Python 3.x** and `pybind11`.
* **OpenMP** support.

### Compilation
1. Clone the repository:
   ```bash
   git clone https://github.com/Tzintzun444/montecarlo-engine.git
   cd montecarlo-engine
2. Create a build directory and configure with CMake:
   ```bash
   mkdir build && cd build 
   cmake ..
3. Compile the module:
   ```bash
   cmake --build . --config Release
4. The resulting `.so` (Linux/Mac) or `.pyd` (Windows) binary will be generated. Place it in your Python project directory to import it.

## Usage Example (Python)
Once compiled, the engine acts as a native Python module:
```python
import montecarlo_engine
import time

# Financial Parameters
S_0 = 100.0       # Spot Price
vol = 0.20        # Volatility (20%)
K = 100.0         # Strike Price
T = 1.0           # Time to Maturity (Years)
r = 0.05          # Risk-free rate (5%)
OT = montecarlo_engine.OptionType.Call
N = 1_000_000     # Number of simulations

print(f"Running Montecarlo with N = {N}...")
start = time.perf_counter()

# C++ Engine Execution
res = montecarlo_engine.run_montecarlo(S_0, vol, K, T, r, OT, N)

end = time.perf_counter()

print(f"Fair Value: {res.fairValue:.6f}")
print(f"Confidence Interval: [{res.CI_lower:.6f}, {res.CI_upper:.6f}]")
print(f"Delta: {res.delta:.6f} | Gamma: {res.gamma:.6f} | Vega: {res.vega:.6f}")
print(f"Execution Time: {(end - start) * 1000:.2f} ms")
```

## Project Structure
* `src/montecarlo/`: Core pricing engine, memory handling, and mathematical logic.
* `src/xoshiro256/`: PRNG implementation.
* `bindings/`: `pybind11` wrapper exposing C++ structs and properties to Python as dictionaries and attributes

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/Tzintzun444/montecarlo-engine/blob/main/LICENSE) file for details