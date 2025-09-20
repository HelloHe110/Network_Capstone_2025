# Lab 3: Network Optimization - Satellite-Ground Station Assignment

## Overview

This lab implements advanced network optimization algorithms for satellite-ground station assignment problems. The project focuses on solving the optimal assignment of ground stations to satellites to minimize data collection time using multiple optimization approaches including greedy algorithms, linear programming, and bipartite matching.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 3 - Network Optimization
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Network Graph Generation**: Create realistic satellite-ground station network topologies
2. **Greedy Algorithm Implementation**: Develop heuristic solutions for assignment problems
3. **Linear Programming Optimization**: Implement optimal solutions using OR-Tools
4. **Bipartite Matching**: Solve maximum weight bipartite matching problems
5. **Performance Comparison**: Evaluate different optimization approaches
6. **Beamforming Integration**: Incorporate antenna array processing for realistic link rates

## Problem Formulation

### Satellite-Ground Station Assignment Problem

Given:
- **V** ground stations with known positions
- **S** satellites with known positions  
- **L** potential links with data rates based on beamforming

Find:
- Optimal assignment of each ground station to exactly one satellite
- Minimize maximum data collection time across all satellites

### Mathematical Model

**Variables:**
- `x[v][s]` = 1 if ground station v is assigned to satellite s, 0 otherwise

**Constraints:**
- Each ground station assigned to exactly one satellite: `∑_s x[v][s] = 1` for all v
- Data collection time: `T ≥ ∑_v x[v][s] * t[v][s]` for all s

**Objective:**
- Minimize maximum collection time: `min T`

## Implementation Details

### Core Files

#### Main Optimization Algorithms
- **`lab3_greedy.cc`**: Greedy heuristic algorithm implementation
- **`lab3_ortools.cc`**: Linear programming solution using Google OR-Tools
- **`lab3_bipartite.cc`**: Bipartite matching algorithm implementation
- **`basic_example.cc`**: OR-Tools basic example and setup

#### Network Generation
- **`bf.m`**: MATLAB script for network graph generation with beamforming
- **`lab2_bf_reference.m`**: Reference beamforming implementation
- **`ewa_function/`**: Essential wave analysis functions for antenna arrays

#### Data Files
- **`network.graph`**: Generated network topology with link rates
- **`network.pos`**: Ground station and satellite positions
- **`sample.graph`**: Sample network for testing
- **`network.greedy.out`**: Greedy algorithm output
- **`network.ortools.out`**: OR-Tools optimization output

### Key Algorithms

#### 1. Greedy Algorithm
```cpp
// For each ground station, select satellite with maximum rate
for (int v = 0; v < V; ++v) {
    double best_rate = 0.0;
    int best_sat = -1;
    
    for (int s = 0; s < S; ++s) {
        if (rate[v][s] > best_rate) {
            best_rate = rate[v][s];
            best_sat = s;
        }
    }
    assignment[v] = best_sat;
}
```

#### 2. Linear Programming (OR-Tools)
```cpp
// Create decision variables
std::vector<std::vector<const MPVariable*>> x(V);
for (int v = 0; v < V; ++v) {
    for (int k = 0; k < adj[v].size(); ++k) {
        x[v].push_back(solver->MakeBoolVar("x_" + std::to_string(v) + "_" + std::to_string(s)));
    }
}

// Assignment constraints
for (int v = 0; v < V; ++v) {
    MPConstraint* c = solver->MakeRowConstraint(1.0, 1.0);
    for (auto* var : x[v]) c->SetCoefficient(var, 1.0);
}

// Time constraints
const MPVariable* T = solver->MakeNumVar(0.0, MPSolver::infinity(), "T");
for (int s = 0; s < S; ++s) {
    MPConstraint* c = solver->MakeRowConstraint(0.0, MPSolver::infinity());
    c->SetCoefficient(T, 1.0);
    // Add satellite load constraints
}
```

#### 3. Bipartite Matching
```cpp
// Weight matrix for bipartite graph
std::vector<std::vector<int>> weights = {
    {5, 8, 6},
    {4, 7, 2}, 
    {3, 9, 1}
};

// Create variables for each potential match
for (int i = 0; i < left_num; ++i) {
    for (int j = 0; j < right_num; ++j) {
        x[i][j] = solver->MakeIntVar(0, 1, "x_" + std::to_string(i) + std::to_string(j));
    }
}

// Matching constraints
for (int i = 0; i < left_num; ++i) {
    LinearExpr sum;
    for (int j = 0; j < right_num; ++j) sum += x[i][j];
    solver->MakeRowConstraint(sum <= 1);
}
```

### Network Graph Generation

#### Beamforming-Based Link Rates
```matlab
% Calculate 3D distance and elevation angle
distance = norm(sat_pos - gs_pos);
elevation = atan2d(horizontal_dist, height_diff);

% Compute beamforming gain
[weights, ~] = uniform(d, elevation, tx_antenna_number);
gain = abs(dtft(weights, -psi)).^2;

% Calculate link rate using Friis equation
pathloss = friis_equation(freq, gain, 1, distance);
rate = bandwidth_Hz * log2(1 + 10^((P_tx_dBm + pathloss - N0_dBm)/10));
```

#### Graph Format
```
V S L                    # V ground stations, S satellites, L links
v s rate                 # Ground station v to satellite s with data rate
...
```

## Lab Tasks

### Task 1: Network Graph Generation
- Generate realistic satellite-ground station network topologies
- Calculate link rates using beamforming and propagation models
- Create network graph files for optimization algorithms

### Task 2: Greedy Algorithm Implementation
- Implement greedy assignment strategy
- Select best satellite for each ground station
- Calculate maximum collection time

### Task 3: Linear Programming Optimization
- Formulate assignment problem as linear program
- Implement using Google OR-Tools
- Solve for optimal assignment

### Task 4: Bipartite Matching
- Implement maximum weight bipartite matching
- Compare with other optimization approaches
- Analyze solution quality and performance

### Task 5: Performance Analysis
- Compare greedy vs optimal solutions
- Analyze computational complexity
- Evaluate solution quality metrics

## Usage Instructions

### Prerequisites
- **C++ Compiler**: GCC 7.0+ or compatible
- **Google OR-Tools**: Linear programming library
- **MATLAB**: For network generation and beamforming
- **CMake**: Build system

### Building the Project
```bash
# Navigate to lab directory
cd lab3-HelloHe110

# Build OR-Tools examples
mkdir build
cmake -S. -Bbuild
cmake --build build -v

# Compile greedy algorithm
g++ -o lab3_greedy lab3_greedy.cc

# Compile OR-Tools optimization
g++ -o lab3_ortools lab3_ortools.cc -lortools

# Compile bipartite matching
g++ -o lab3_bipartite lab3_bipartite.cc -lortools
```

### Running the Algorithms
```bash
# Generate network graph
matlab -r "run('bf.m')"

# Run greedy algorithm
./lab3_greedy network.graph

# Run OR-Tools optimization
./lab3_ortools network.graph

# Run bipartite matching
./lab3_bipartite

# Run basic OR-Tools example
./build/bin/BasicExample
```

### Memory Management
```bash
# Clean and rebuild (if memory issues)
chmod +x mem.sh
./mem.sh
```

## Algorithm Comparison

### Greedy Algorithm
- **Complexity**: O(V × S)
- **Solution Quality**: Heuristic, not optimal
- **Advantages**: Fast, simple implementation
- **Disadvantages**: May not find optimal solution

### Linear Programming (OR-Tools)
- **Complexity**: Exponential worst-case, polynomial average
- **Solution Quality**: Optimal
- **Advantages**: Guaranteed optimal solution
- **Disadvantages**: Higher computational cost

### Bipartite Matching
- **Complexity**: O(V³) for maximum weight matching
- **Solution Quality**: Optimal for matching problems
- **Advantages**: Specialized for assignment problems
- **Disadvantages**: Limited to specific problem types

## Results and Analysis

### Output Files
- **`network.greedy.out`**: Greedy algorithm results
- **`network.ortools.out`**: OR-Tools optimization results
- **Performance metrics**: Solution quality and execution time

### Key Metrics
- **Collection Time**: Maximum time across all satellites
- **Assignment Quality**: How close to optimal
- **Computational Time**: Algorithm execution time
- **Memory Usage**: Resource consumption

## Technical Achievements

### Optimization Algorithms
- **Multiple Approaches**: Greedy, LP, and bipartite matching
- **Performance Analysis**: Comprehensive algorithm comparison
- **Real-world Integration**: Beamforming-based link rates

### Network Modeling
- **Realistic Topologies**: 3D satellite and ground station positions
- **Propagation Modeling**: Friis equation with beamforming
- **Dynamic Link Rates**: Position-dependent data rates

### Software Engineering
- **Modular Design**: Separate algorithms and utilities
- **Cross-platform**: C++ and MATLAB integration
- **Professional Tools**: Google OR-Tools integration

## Advanced Features

### Beamforming Integration
- **Antenna Array Processing**: Uniform linear arrays
- **Directional Communication**: Elevation angle optimization
- **Realistic Link Rates**: Physics-based calculations

### Scalability Analysis
- **Large Networks**: Support for many stations and satellites
- **Performance Profiling**: Computational complexity analysis
- **Memory Optimization**: Efficient data structures

### Research Applications
- **Algorithm Development**: New optimization approaches
- **Performance Evaluation**: Comparative studies
- **Network Planning**: Satellite constellation optimization

## Future Enhancements

- **Machine Learning**: AI-based assignment strategies
- **Dynamic Optimization**: Time-varying assignments
- **Multi-objective**: Consider multiple optimization criteria
- **Distributed Algorithms**: Parallel processing approaches

## Acknowledgments

This lab builds upon the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and utilizes Google OR-Tools for optimization.

Special thanks to:
- The NYCU Network Capstone course instructors
- Google OR-Tools development team
- The optimization research community

## References

1. Google OR-Tools Documentation
2. Linear Programming and Network Flows
3. Bipartite Matching Algorithms
4. Satellite Communication Systems

---

*This lab demonstrates advanced concepts in network optimization, linear programming, and algorithm design. All implementations are original work developed as part of the Network Capstone course requirements.*