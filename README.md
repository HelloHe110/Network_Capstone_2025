# Network Capstone 2025 - LEO Satellite Communication System

## Overview

This repository contains the complete implementation of a Low Earth Orbit (LEO) satellite communication system developed as part of the Network Capstone course at NYCU. The project focuses on satellite swarm communication protocols, beamforming techniques, and network optimization algorithms.

## Student Information

- **Student ID**: 111550022
- **Course**: Network Capstone 2025
- **Institution**: National Yang Ming Chiao Tung University (NYCU)

## Project Structure

The project is organized into six main laboratory assignments, each building upon the previous one:

### Lab 0: HelloHe110
- **Purpose**: Initial setup and basic solver implementation
- **Files**: `mysolver.m`, `solveX.m`
- **Description**: Introduction to MATLAB-based satellite communication simulation

### Lab 1: Beamforming Fundamentals
- **Purpose**: Basic beamforming algorithms and antenna array processing
- **Key Files**: 
  - `bf.m` - Main beamforming implementation
  - `bf_modify.m` - Modified beamforming algorithm
  - `ewa_function/` - Essential wave analysis functions
- **Features**: 
  - Friis equation implementation
  - Direction of arrival (DOA) estimation
  - Uniform linear array processing

### Lab 2: LEO Satellite Network Simulation
- **Purpose**: Complete LEO satellite constellation simulation using ns-3
- **Key Components**:
  - Satellite orbit modeling
  - Inter-satellite link (ISL) simulation
  - Ground station communication
  - Propagation loss models
- **Features**:
  - Support for Starlink and Telesat constellations
  - Real-time satellite position tracking
  - Network topology visualization

### Lab 3: Network Optimization
- **Purpose**: Graph-based network optimization and routing algorithms
- **Key Files**:
  - `lab3_bipartite.cc` - Bipartite graph matching
  - `lab3_greedy.cc` - Greedy optimization algorithm
  - `lab3_ortools.cc` - Google OR-Tools integration
- **Features**:
  - Network graph analysis
  - Optimization algorithm comparison
  - Performance benchmarking

### Lab 4: Advanced LEO Simulation
- **Purpose**: Enhanced LEO network simulation with optimization
- **Key Files**:
  - `leo-lab4.cc` - Advanced LEO simulation
  - Network topology files
- **Features**:
  - Multi-constellation support
  - Advanced routing protocols
  - Performance analysis

### Lab 5: Custom Algorithm Implementation
- **Purpose**: Development of custom optimization algorithms
- **Key Files**:
  - `lab5_myalgo.cpp` - Custom algorithm implementation
  - `lab5_ortools.cc` - OR-Tools baseline comparison
- **Features**:
  - Algorithm performance comparison
  - Custom optimization strategies
  - Benchmarking against industry standards

### Lab 6: Nix-Vector Routing
- **Purpose**: Implementation of nix-vector routing for satellite networks
- **Key Files**:
  - `leo-lab6.cc` - Main simulation file
  - `nix-vector-routing/` - Custom routing implementation
- **Features**:
  - Efficient routing in dynamic topologies
  - Path computation algorithms
  - Network scalability analysis

## Technical Features

### Satellite Communication
- **Orbit Modeling**: Circular orbit mobility models for LEO satellites
- **Propagation Models**: Realistic path loss calculations for space-ground links
- **Constellation Support**: Starlink, Telesat, and custom constellations

### Network Optimization
- **Graph Algorithms**: Bipartite matching, greedy optimization
- **Routing Protocols**: Nix-vector routing for dynamic topologies
- **Performance Analysis**: Comprehensive benchmarking and evaluation

### Simulation Framework
- **ns-3 Integration**: Full ns-3 network simulator support
- **MATLAB Integration**: Signal processing and analysis tools
- **Visualization**: Network topology and performance visualization

## Dependencies

### Required Software
- **ns-3**: Network simulator (version 3.30+)
- **MATLAB**: Signal processing and analysis
- **C++ Compiler**: GCC 7.0+ or compatible
- **Google OR-Tools**: Optimization library

### Python Libraries (for analysis)
- NumPy
- Matplotlib
- Pandas

## Installation and Usage

### Prerequisites
1. Install ns-3 network simulator
2. Install MATLAB with Signal Processing Toolbox
3. Install Google OR-Tools
4. Ensure C++ compiler is available

### Building the Project
```bash
# Navigate to the project directory
cd Network_Capstone_2025

# Build ns-3 modules (for labs 2-6)
cd lab2-HelloHe110
./waf configure
./waf build

# Run simulations
./waf --run "leo-delay --orbitFile=data/orbits/starlink.csv"
```

### Running MATLAB Scripts
```matlab
% Navigate to lab directory
cd lab1-HelloHe110

% Run beamforming simulation
run('bf.m')

% Run modified beamforming
run('bf_modify.m')
```

## Results and Reports

Each lab includes:
- **Implementation files**: Source code and scripts
- **Report**: Detailed analysis and results (PDF format)
- **Results**: Output data and visualizations
- **Documentation**: README files with specific instructions

## Key Achievements

1. **Complete LEO Simulation**: Implemented a full-featured LEO satellite communication system
2. **Algorithm Development**: Created custom optimization algorithms for satellite networks
3. **Performance Analysis**: Comprehensive evaluation of different approaches
4. **Network Optimization**: Advanced routing and resource allocation strategies

## Future Work

- Integration of machine learning for predictive routing
- Support for additional satellite constellations
- Real-time simulation capabilities
- Enhanced visualization tools

## Acknowledgments

This project is based on the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and builds upon the ns-3-LEO module originally developed by the research community.

Special thanks to:
- The NYCU Network Capstone course instructors
- The ns-3 development community
- The LEO satellite communication research community

## License

This project is developed for educational purposes as part of the Network Capstone course at NYCU. Please refer to individual lab directories for specific licensing information.

## Contact

For questions about this project, please contact:
- Student ID: 111550022
- Course: Network Capstone 2025, NYCU

---

*This project demonstrates advanced concepts in satellite communication, network optimization, and simulation techniques. All implementations are original work developed as part of the Network Capstone course requirements.*
