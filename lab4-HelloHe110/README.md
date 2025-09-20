# Lab 4: Advanced LEO Simulation - Real-time Satellite Communication

## Overview

This lab implements an advanced Low Earth Orbit (LEO) satellite communication system with real-time data collection and transmission scheduling. The project focuses on integrating optimization algorithms from Lab 3 with ns-3 LEO simulation to create a comprehensive satellite network simulation with dynamic data rates, TCP-based communication, and realistic performance evaluation.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 4 - Advanced LEO Simulation
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Real-time Simulation Integration**: Combine optimization algorithms with ns-3 LEO simulation
2. **Dynamic Data Rate Management**: Implement position-dependent link rates
3. **TCP-based Communication**: Realistic transport layer simulation
4. **Performance Evaluation**: Compare greedy vs optimal assignment strategies
5. **Satellite Resource Management**: Handle multiple ground station assignments per satellite
6. **End-to-end Analysis**: Complete data collection time measurement

## Technical Architecture

### System Components

#### Core Simulation Framework
- **ns-3 LEO Module**: Satellite constellation simulation
- **TCP Communication**: Reliable data transmission
- **AODV Routing**: Dynamic routing in satellite networks
- **Optimization Integration**: Lab 3 assignment algorithms

#### Satellite Network Configuration
- **Constellation**: 60 satellites in single orbital plane (1200km altitude, 20° inclination)
- **Ground Stations**: 20 strategically placed ground stations
- **Data Rates**: Position-dependent rates from optimization algorithms
- **Communication Protocol**: TCP with bulk data transfer

### Key Features

#### 1. Real-time Data Collection
```cpp
// Parse optimization results for satellite assignments
void ParseAssociation(string filename) {
    ifstream in(ns3_path_head + filename);
    double dummy;
    in >> dummy;  // Skip total time
    int gs, sat, cnt = 0;
    while (in >> gs >> sat) {
        if (cnt >= 20) break;
        satToGsQueue[sat].push_back(gs);
        assocMap[sat].push_back(gs);
        gsStarted[gs] = false;
    }
}
```

#### 2. Dynamic Link Rate Management
```cpp
// Set data rates based on optimization results
void SendPacket(int gsId, int satId) {
    // Update data rates for both ground station and satellite
    utNet.Get(gsNode->GetId())->GetObject<MockNetDevice>()->SetDataRate(
        linkRates[make_pair(gsId, satId)]);
    utNet.Get(satNode->GetId())->GetObject<MockNetDevice>()->SetDataRate(
        linkRates[make_pair(gsId, satId)]);
    
    // Create TCP bulk send application
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(dstAddr, port));
    source.SetAttribute("MaxBytes", UintegerValue(SIZEEE));
    source.SetAttribute("SendSize", UintegerValue(512));
}
```

#### 3. Satellite Resource Management
```cpp
// Handle multiple ground stations per satellite
static void EchoRx(std::string context, const Ptr<const Packet> packet, 
                   const TcpHeader &header, const Ptr<const TcpSocketBase> socket) {
    // Check if satellite finished receiving data
    if (totalRx < (uint64_t)(SIZEEE * (sat_cnt[nodeId]))) return;
    
    // Schedule next ground station for this satellite
    for (int gsId : assocMap[nodeId]) {
        if (gsStartTime.find(gsId) == gsStartTime.end()) {
            Simulator::ScheduleNow(&SendPacket, gsId, nodeId);
            break;
        }
    }
}
```

## Implementation Details

### Core Files

#### Main Simulation File
- **`leo-lab4.cc`**: Complete LEO simulation with optimization integration
- **Network Configuration**: 60 satellites, 20 ground stations
- **TCP Communication**: Bulk data transfer applications
- **Performance Monitoring**: Real-time data collection tracking

#### Input Files
- **`network.graph`**: Ground station-satellite link rates
- **`network.ortools.out`**: Optimal assignment from Lab 3
- **`network.greedy.out`**: Greedy assignment from Lab 3

#### Output Files
- **`lab4.ortools.out`**: Simulation results with optimal assignment
- **`lab4.greedy.out`**: Simulation results with greedy assignment
- **Performance Metrics**: Data collection times and satellite utilization

### Key Algorithms

#### 1. Satellite Constellation Setup
```cpp
// Create 60-satellite constellation
LeoOrbitNodeHelper orbit;
satellites = orbit.Install({LeoOrbit(1200, 20, 1, 60)});

// Add 20 ground stations at specific locations
LeoGndNodeHelper ground;
ground.Add(groundStations, LeoLatLong(20, 4));
ground.Add(groundStations, LeoLatLong(19, 12));
// ... additional ground stations
```

#### 2. Network Configuration
```cpp
// Configure LEO channels with Telesat constellation
LeoChannelHelper utCh;
utCh.SetConstellation("TelesatGateway");
utNet = utCh.Install(satellites, groundStations);

// Setup AODV routing
AodvHelper aodv;
aodv.Set("EnableHello", BooleanValue(false));
InternetStackHelper stack;
stack.SetRoutingHelper(aodv);
```

#### 3. Data Collection Scheduling
```cpp
// Initialize data collection for each satellite
for (auto &[sat, q] : satToGsQueue) {
    if (!q.empty() && !gsStarted[q.front()]) {
        SendPacket(q.front(), sat);
    }
}

// Connect TCP receive callback
Simulator::Schedule(Seconds(1e-7), &Connect);
```

## Lab Tasks

### Task 1: Simulation Integration
- Integrate Lab 3 optimization results with ns-3 LEO simulation
- Implement dynamic data rate management
- Setup TCP-based communication protocols

### Task 2: Real-time Performance Monitoring
- Track data collection times for each ground station
- Monitor satellite utilization and resource allocation
- Implement end-to-end performance measurement

### Task 3: Algorithm Comparison
- Compare greedy vs optimal assignment strategies
- Analyze data collection time differences
- Evaluate satellite resource utilization

### Task 4: Network Optimization
- Implement efficient satellite-ground station scheduling
- Handle multiple assignments per satellite
- Optimize data transmission order

## Performance Analysis

### Simulation Results

#### Optimal Assignment (OR-Tools)
- **Total Collection Time**: 0.278296 seconds
- **Satellite Utilization**: Varied across satellites (0-0.26545 seconds)
- **Ground Station Performance**: Efficient data transmission scheduling

#### Greedy Assignment
- **Total Collection Time**: 0.682102 seconds
- **Satellite Utilization**: Less balanced distribution
- **Performance Gap**: ~2.45x slower than optimal

### Key Metrics

#### Data Collection Efficiency
- **Optimal Strategy**: 0.278 seconds total time
- **Greedy Strategy**: 0.682 seconds total time
- **Improvement**: 59% reduction in collection time

#### Satellite Resource Utilization
- **Load Balancing**: Optimal assignment provides better load distribution
- **Resource Efficiency**: More even utilization across satellite constellation
- **Throughput**: Higher overall network throughput

## Technical Achievements

### Advanced Simulation
- **Real-time Integration**: Seamless optimization algorithm integration
- **Dynamic Configuration**: Position-dependent data rate management
- **TCP Implementation**: Reliable transport layer simulation

### Network Optimization
- **Algorithm Comparison**: Comprehensive performance evaluation
- **Resource Management**: Efficient satellite resource allocation
- **Scalability**: Support for large satellite constellations

### Performance Analysis
- **End-to-end Metrics**: Complete data collection time measurement
- **Statistical Analysis**: Detailed performance comparison
- **Visualization**: Clear performance difference demonstration

## Usage Instructions

### Prerequisites
- **ns-3**: Version 3.30+ with LEO module
- **C++ Compiler**: GCC 7.0+ or compatible
- **Lab 3 Results**: Optimization output files required

### Building the Project
```bash
# Navigate to lab directory
cd lab4-HelloHe110

# Ensure Lab 3 optimization files are present
ls network.ortools.out network.greedy.out

# Build ns-3 simulation
./waf configure --enable-examples
./waf build
```

### Running the Simulation
```bash
# Run with optimal assignment
./waf --run "leo-lab4 --inputFile=network.ortools.out --outputFile=lab4.ortools.out"

# Run with greedy assignment
./waf --run "leo-lab4 --inputFile=network.greedy.out --outputFile=lab4.greedy.out"

# Compare results
diff lab4.ortools.out lab4.greedy.out
```

### Configuration Options
```bash
# Custom simulation parameters
./waf --run "leo-lab4 \
  --duration=100 \
  --constellation=TelesatGateway \
  --inputFile=network.ortools.out \
  --outputFile=results.out"
```

## Advanced Features

### Dynamic Rate Management
- **Position-dependent Rates**: Link rates based on satellite-ground station geometry
- **Real-time Updates**: Dynamic rate adjustment during simulation
- **Optimization Integration**: Seamless integration with Lab 3 results

### TCP Communication
- **Reliable Transmission**: TCP-based data transfer
- **Bulk Data Transfer**: Efficient large data transmission
- **Congestion Control**: Realistic network behavior

### Performance Monitoring
- **Real-time Tracking**: Live performance monitoring
- **Statistical Analysis**: Comprehensive performance metrics
- **Comparison Tools**: Side-by-side algorithm comparison

## Research Applications

### Satellite Network Optimization
- **Assignment Algorithms**: Advanced optimization strategies
- **Resource Management**: Efficient satellite resource allocation
- **Performance Evaluation**: Comprehensive algorithm comparison

### Network Simulation
- **Realistic Modeling**: Physics-based satellite communication
- **Protocol Analysis**: TCP performance in satellite networks
- **Scalability Studies**: Large-scale constellation simulation

## Future Enhancements

- **Machine Learning**: AI-based assignment optimization
- **Multi-objective Optimization**: Consider multiple performance criteria
- **Dynamic Topology**: Time-varying satellite positions
- **Advanced Protocols**: 5G-NR satellite communication

## Acknowledgments

This lab builds upon the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and integrates with the ns-3-LEO module and Lab 3 optimization algorithms.

Special thanks to:
- The NYCU Network Capstone course instructors
- The ns-3 development community
- The LEO satellite communication research community

## References

1. ns-3 Network Simulator Documentation
2. LEO Satellite Communication Systems
3. TCP Performance in Satellite Networks
4. Network Optimization Algorithms

---

*This lab demonstrates advanced concepts in satellite network simulation, optimization integration, and performance evaluation. All implementations are original work developed as part of the Network Capstone course requirements.*