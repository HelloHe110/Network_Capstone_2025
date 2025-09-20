# Lab 5: Custom Algorithm Development - Maximum Sum Throughput Optimization

## Overview

This lab implements custom optimization algorithms for the Maximum Sum Throughput problem in wireless communication networks. The project focuses on developing both heuristic and optimal solutions for multi-flow routing with capacity constraints, single-transmit/single-receive limitations, and path-based flow allocation.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 5 - Custom Algorithm Development
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Custom Algorithm Design**: Develop heuristic solution for maximum throughput routing
2. **Linear Programming Formulation**: Implement optimal solution using OR-Tools
3. **Multi-flow Routing**: Handle multiple source-destination pairs simultaneously
4. **Constraint Handling**: Implement capacity and single-transmit/receive constraints
5. **Performance Comparison**: Evaluate custom algorithm against optimal solution
6. **Algorithm Analysis**: Analyze computational complexity and solution quality

## Problem Formulation

### Maximum Sum Throughput Problem

Given:
- **Network Graph**: G = (V, E) with edge capacities
- **Flow Requests**: F source-destination pairs
- **Constraints**: 
  - Edge capacity limits
  - Single transmit per node
  - Single receive per node
  - Path-based routing (no cycles)

Find:
- Maximum total throughput across all flows
- Valid routing paths for each flow
- Optimal resource allocation

### Mathematical Model

**Variables:**
- `X[f,u,v]`: Flow f on edge (u,v) (continuous)
- `Y[f,u,v]`: Flow f uses edge (u,v) (binary)
- `Z[u,v]`: Edge (u,v) is used by any flow (binary)

**Constraints:**
- Flow conservation: `∑_v X[f,u,v] - ∑_v X[f,v,u] = 0` for intermediate nodes
- Edge capacity: `∑_f X[f,u,v] ≤ capacity[u,v]`
- Single transmit: `∑_v Z[u,v] ≤ 1` for each node u
- Single receive: `∑_v Z[v,u] ≤ 1` for each node u
- Path constraint: `X[f,u,v] ≤ Y[f,u,v] × capacity[u,v]`

**Objective:**
- Maximize total throughput: `max ∑_f X[f,source_f,dest_f]`

## Implementation Details

### Core Files

#### Custom Algorithm Implementation
- **`lab5_myalgo.cpp`**: Heuristic greedy algorithm with Dijkstra-based path finding
- **`lab5_ortools.cc`**: Optimal linear programming solution using OR-Tools
- **Input/Output**: Standard format for network graphs and flow requests

#### Test Data
- **`network.graph`**: Large-scale network test case
- **`sample.graph`**: Sample test cases for validation
- **Output Files**: Algorithm results and performance metrics

### Key Algorithms

#### 1. Custom Heuristic Algorithm
```cpp
// Greedy approach with capacity-aware path finding
bool find_path(const Instance& ins, vector<Edge>& edges,
               const vector<bool>& tx_used, const vector<bool>& rx_used,
               int s, int d, vector<int>& path, double& bottleneck) {
    // Dijkstra with capacity-aware weights
    priority_queue<pdi, vector<pdi>, greater<>> pq;
    vector<double> dist(n, 1e18);
    
    // Weight function considers capacity utilization
    double weight = 1.0 + e.used * 1e4 / max(e.capacity, 1.0);
    
    // Find minimum cost path respecting constraints
    while (!pq.empty()) {
        auto [cost, u] = pq.top(); pq.pop();
        if (u == d) break;
        
        for (int eid : ins.adj[u]) {
            auto& e = edges[eid];
            if (e.used >= e.capacity) continue;
            if (tx_used[e.from] || rx_used[e.to]) continue;
            
            // Update distance and path
            if (dist[e.to] > dist[u] + weight) {
                dist[e.to] = dist[u] + weight;
                prev_edge[e.to] = eid;
                pq.emplace(dist[e.to], e.to);
            }
        }
    }
}
```

#### 2. Linear Programming Solution
```cpp
// OR-Tools-based optimal solution
double SolveSingleTestCase(int U, int E, int F, istream& in, ostream& out) {
    // Create MILP solver
    MPSolver solver("ILP", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);
    
    // Decision variables
    map<tuple<int, int, int>, const MPVariable*> X;  // Flow variables
    map<tuple<int, int, int>, const MPVariable*> Y;  // Path variables
    map<pair<int, int>, const MPVariable*> Z;        // Edge usage variables
    
    // Objective: maximize total throughput
    MPObjective* objective = solver.MutableObjective();
    for (int f = 0; f < F; ++f) {
        auto key = make_tuple(f, virSrcs[f], sources[f]);
        if (X.find(key) != X.end()) {
            objective->SetCoefficient(X[key], 1.0);
        }
    }
    objective->SetMaximization();
    
    // Constraints
    // 1. Flow conservation
    // 2. Edge capacity limits
    // 3. Single transmit/receive constraints
    // 4. Path-based routing constraints
}
```

#### 3. Path Reconstruction
```cpp
// Reconstruct optimal paths from LP solution
vector<int> reconstruct_path(int f, int s, int d, 
                           const map<tuple<int,int,int>, const MPVariable*>& X) {
    unordered_map<int, int> parent;
    queue<int> q;
    q.push(s);
    parent[s] = -1;
    
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == d) break;
        
        for (int v = 0; v < V; ++v) {
            auto it = X.find({f, u, v});
            if (it != X.end() && it->second->solution_value() > 1e-6) {
                parent[v] = u;
                q.push(v);
            }
        }
    }
    
    // Reconstruct path from destination to source
    vector<int> path;
    int curr = d;
    while (curr != -1) {
        path.push_back(curr);
        curr = parent[curr];
    }
    reverse(path.begin(), path.end());
    return path;
}
```

## Lab Tasks

### Task 1: Custom Algorithm Development
- Implement greedy heuristic with capacity-aware path finding
- Handle single transmit/receive constraints
- Develop efficient path reconstruction algorithm

### Task 2: Linear Programming Formulation
- Formulate multi-flow routing as mixed-integer linear program
- Implement constraint generation for all problem constraints
- Integrate with OR-Tools solver

### Task 3: Performance Analysis
- Compare custom algorithm vs optimal solution
- Analyze computational complexity
- Evaluate solution quality metrics

### Task 4: Algorithm Optimization
- Optimize custom algorithm performance
- Implement efficient data structures
- Reduce computational overhead

## Algorithm Comparison

### Custom Heuristic Algorithm
- **Complexity**: O(F × (E + V log V)) per flow
- **Solution Quality**: Heuristic, not guaranteed optimal
- **Advantages**: Fast execution, simple implementation
- **Disadvantages**: May not find optimal solution

### Linear Programming (OR-Tools)
- **Complexity**: Exponential worst-case, polynomial average
- **Solution Quality**: Optimal
- **Advantages**: Guaranteed optimal solution
- **Disadvantages**: Higher computational cost

### Performance Results

#### Sample Test Case
- **Custom Algorithm**: 8.000000 total throughput
- **OR-Tools Solution**: 9.000000 total throughput
- **Performance Gap**: 12.5% improvement with optimal solution

#### Network Test Case
- **Custom Algorithm**: Multiple flows with varying performance
- **OR-Tools Solution**: Optimal allocation across all flows
- **Scalability**: Both algorithms handle large networks

## Technical Achievements

### Custom Algorithm Design
- **Greedy Strategy**: Capacity-aware path selection
- **Constraint Handling**: Single transmit/receive limitations
- **Path Finding**: Dijkstra-based routing with capacity weights

### Linear Programming
- **MILP Formulation**: Mixed-integer linear programming model
- **Constraint Modeling**: Comprehensive constraint generation
- **Optimal Solution**: Guaranteed optimal throughput

### Performance Analysis
- **Algorithm Comparison**: Side-by-side performance evaluation
- **Scalability Testing**: Large network performance analysis
- **Solution Quality**: Throughput optimization metrics

## Usage Instructions

### Prerequisites
- **C++ Compiler**: GCC 7.0+ or compatible
- **Google OR-Tools**: Linear programming library
- **Standard Libraries**: STL containers and algorithms

### Building the Project
```bash
# Navigate to lab directory
cd lab5-HelloHe110

# Compile custom algorithm
g++ -o lab5_myalgo lab5_myalgo.cpp

# Compile OR-Tools solution
g++ -o lab5_ortools lab5_ortools.cc -lortools
```

### Running the Algorithms
```bash
# Run custom algorithm
./lab5_myalgo < network.graph > network.myalgo.out

# Run OR-Tools solution
./lab5_ortools < network.graph > network.ortools.out

# Compare results
diff network.myalgo.out network.ortools.out
```

### Input Format
```
T                    # Number of test cases
U E F               # Nodes, Edges, Flows
u v capacity        # Edge definitions
s d                 # Source-destination pairs
```

### Output Format
```
student_id          # Student identifier
used_edges_count    # Number of used edges
u v                 # Used edge list
rate path_length path_nodes  # Flow routing information
total_throughput    # Sum of all flow rates
```

## Advanced Features

### Custom Algorithm Optimizations
- **Capacity-aware Weights**: Path selection considers capacity utilization
- **Constraint Pruning**: Early elimination of invalid paths
- **Efficient Data Structures**: Optimized graph representation

### Linear Programming Enhancements
- **Virtual Node Technique**: Simplified flow modeling
- **Constraint Tightening**: Improved LP formulation
- **Path Reconstruction**: Efficient solution interpretation

### Performance Monitoring
- **Throughput Tracking**: Real-time performance metrics
- **Resource Utilization**: Edge and node usage analysis
- **Algorithm Comparison**: Comprehensive performance evaluation

## Research Applications

### Network Optimization
- **Multi-flow Routing**: Advanced routing algorithms
- **Resource Allocation**: Optimal capacity utilization
- **Constraint Optimization**: Complex constraint handling

### Algorithm Development
- **Heuristic Design**: Custom optimization strategies
- **Performance Analysis**: Algorithm comparison studies
- **Scalability Research**: Large-scale network optimization

## Future Enhancements

- **Machine Learning**: AI-based path selection
- **Dynamic Routing**: Time-varying network conditions
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
2. Network Flow Algorithms
3. Linear Programming and Network Optimization
4. Multi-commodity Flow Problems

---

*This lab demonstrates advanced concepts in custom algorithm development, linear programming, and network optimization. All implementations are original work developed as part of the Network Capstone course requirements.*