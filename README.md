# CVRP High-Performance Genetic Algorithm Solver (C++20)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20+-064F8C.svg)](https://cmake.org)
[![Testing](https://img.shields.io/badge/Tests-GoogleTest-green.svg)](https://github.com/google/googletest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modern, high-performance C++20 implementation of a **Genetic Algorithm (GA)** designed to solve the **Capacitated Vehicle Routing Problem (CVRP / LcVRP)**.

The CVRP is a classic NP-hard combinatorial optimization problem where a fleet of homogeneous vehicles stationed at a central depot must service a set of customers with distinct demands while minimizing total distance traveled and strictly obeying vehicle payload capacity constraints.

---

## Architecture Overview

```
                      +-----------------------------+
                      |   VRP Benchmark Instance    | (.lcvrp / TSPLIB)
                      +--------------+--------------+
                                     |
                                     v
                      +-----------------------------+
                      |       InstanceLoader        |
                      +--------------+--------------+
                                     |
                                     v
                      +-----------------------------+
                      |       ProblemInstance       | (Flattened L1/L2 Distance Matrix)
                      +--------------+--------------+
                                     |
                                     v
                      +-----------------------------+
                      |          Evaluator          | <--- Multi-threaded chunked worker pool
                      +--------------+--------------+
                                     ^
                                     | (Population Evaluation)
                                     v
                      +-----------------------------+
                      |      GeneticAlgorithm       | (Tournament Selection, Crossover,
                      +--------------+--------------+  Swap Mutation, Elitism)
                                     |
                                     v
                      +-----------------------------+
                      |     Optimal Vehicle Routes  | (Detailed trip breakdown & cost)
                      +-----------------------------+
```

---

## Key Features & Engineering Highlights

* **Modern C++20 Standard:** Leverages modern standard features, strongly-typed domain models, `std::filesystem`, move semantics, and `<chrono>` timers.
* **Cache-Locality Optimization:** Flattened 1D distance matrix (`distanceMatrix[i * N + j]`) reducing pointer indirections and cache misses compared to nested `std::vector<std::vector<int>>`.
* **Multi-threaded Parallel Evaluation:** Population fitness evaluation is parallelized across CPU cores using `std::async` and chunked workload partitioning.
* **Elitist Genetic Operators:**
  * **Tournament Selection:** Configurable tournament size to maintain selection pressure.
  * **Single-Point & Permutation Crossover:** Preserves valid gene structures across generations.
  * **Dual Mutation Operators:** Gene-level uniform perturbation and stochastic pairwise gene swaps to prevent premature convergence.
  * **Elitism Preservation:** Guarantees non-decreasing best fitness across generations.
* **Automated Unit Testing:** Comprehensive test coverage via **GoogleTest (GTest)** verifying parsing, feasibility invariants, parallel vs sequential consistency, and solver convergence.
* **CI/CD Integration:** Automated cross-platform GitHub Actions workflow (Ubuntu & Windows).

---

## Directory Structure

```
├── .github/workflows/   # CI/CD GitHub Actions workflow
├── CMakeLists.txt       # Modern target-based CMake build definition
├── README.md            # Project documentation
├── data/                # Sample benchmark instances (A-n32-k5, A-n33-k5, A-n33-k6)
├── include/cvrp/        # Public API headers
│   ├── Evaluator.h
│   ├── GeneticAlgorithm.h
│   ├── Individual.h
│   ├── InstanceLoader.h
│   └── ProblemInstance.h
├── src/                 # Implementation files
│   ├── Evaluator.cpp
│   ├── GeneticAlgorithm.cpp
│   ├── Individual.cpp
│   ├── InstanceLoader.cpp
│   ├── ProblemInstance.cpp
│   └── main.cpp         # CLI application entry point
└── tests/               # GoogleTest unit test suite
    ├── test_algorithm.cpp
    ├── test_evaluator.cpp
    ├── test_individual.cpp
    └── test_loader.cpp
```

---

## Building and Running

### Prerequisites
* CMake 3.20 or newer
* C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 2019+)
* Git

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/gpmainez/cvrp-genetic-solver.git
cd cvrp-genetic-solver

# Generate build configuration
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile all targets (library, solver CLI, test runner)
cmake --build build --config Release
```

### Running Unit Tests

```bash
ctest --test-dir build -C Release --output-on-failure
# Or directly run the binary:
./build/cvrp_tests
```

### Running the Solver CLI

```bash
# Run with default benchmark (data/A-n33-k5.lcvrp)
./build/cvrp_solver

# Run with custom parameters
./build/cvrp_solver --file data/A-n33-k5.lcvrp --pop 150 --gens 1000 --threads 8
```

#### CLI Options

| Flag | Description | Default |
|---|---|---|
| `--file <path>` | Path to `.lcvrp` instance file | `data/A-n33-k5.lcvrp` |
| `--pop <int>` | Population size | `100` |
| `--gens <int>` | Number of generations | `1000` |
| `--cross <float>` | Crossover probability | `0.80` |
| `--mut <float>` | Mutation probability | `0.02` |
| `--threads <int>` | Worker threads (0 = auto) | `0` |
| `--quiet` | Suppress periodic log outputs | Disabled |
| `--help` | Display command-line options | - |

---

## Sample Output

```
=========================================================
   CVRP High-Performance Genetic Algorithm Solver (C++20)
=========================================================
[INFO] Loading instance: data/A-n33-k5.lcvrp ...
[INFO] Instance: A-n33-k5 | Dimension: 33 | Clients: 32 | Vehicles: 5 | Capacity: 100
[INFO] Config: PopSize=100, Gens=500, CrossProb=0.8, MutProb=0.02, Threads=auto

Starting optimization...
[Gen   100/500] Best: 701.00 | Avg: 757.99
[Gen   500/500] Best: 701.00 | Avg: 748.18

====================== RESULTS ==========================
Best Total Cost (Distance): 701.00
Execution Time:            560.13 ms
=========================================================

Vehicle Routes Breakdown:
Vehicle #0 (Distance: 294.00, Total Demand: 183):
  Trip 1 [Load 88/100]: 0 -> 17 -> 9 -> 3 -> 16 -> 29 -> 23 -> 0 (Cost: 185.00)
  Trip 2 [Load 95/100]: 0 -> 28 -> 18 -> 19 -> 14 -> 21 -> 1 -> 31 -> 11 -> 0 (Cost: 109.00)
...
=========================================================
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
