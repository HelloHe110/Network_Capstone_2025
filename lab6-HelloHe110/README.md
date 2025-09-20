# Lab 6: Nix-Vector Routing - Custom Routing Protocol Implementation

## Overview

This lab implements a custom nix-vector routing protocol for satellite communication networks using ns-3. The project focuses on developing efficient routing algorithms for dynamic satellite topologies, implementing path-based routing with pre-computed routes, and analyzing network performance with multiple concurrent flows.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 6 - Nix-Vector Routing
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Custom Routing Protocol**: Implement nix-vector routing for satellite networks
2. **Path-based Routing**: Pre-compute and store routing paths for efficient packet forwarding
3. **Multi-flow Analysis**: Handle multiple concurrent data flows simultaneously
4. **Performance Evaluation**: Measure throughput and packet tracing in satellite networks
5. **Protocol Integration**: Integrate custom routing with ns-3 LEO simulation
6. **Network Optimization**: Optimize routing for satellite constellation topologies

## Technical Architecture

### System Components

#### Core Routing Implementation
- **Nix-Vector Routing**: Custom routing protocol for satellite networks
- **Path Pre-computation**: Pre-calculated routing tables for efficient forwarding
- **LEO Integration**: Seamless integration with satellite constellation simulation
- **TCP Communication**: Reliable data transmission with performance monitoring

#### Satellite Network Configuration
- **Constellation**: 36 satellites (6 planes × 6 satellites per plane)
- **Ground Stations**: 6 ground stations at strategic locations
- **Inter-satellite Links**: Dynamic ISL connections between satellites
- **Ground-Satellite Links**: User terminal connections

### Key Features

#### 1. Nix-Vector Routing Protocol
```cpp
// Custom nix-vector routing implementation
class NixVectorRouting : public Ipv4RoutingProtocol {
    // Path table for pre-computed routes
    std::map<std::pair<int, int>, std::vector<int>> Table;
    
    // Set pre-computed paths from file
    void SetPaths(std::string pathFile);
    
    // Route packets using nix-vector
    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, 
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
};
```

#### 2. Path Pre-computation
```cpp
// Load routing paths from input file
void SetPaths(std::string pathFile) {
    std::ifstream file(pathFile);
    int src, dst, pathLength;
    
    while (file >> src >> dst >> pathLength) {
        std::vector<int> path(pathLength);
        for (int i = 0; i < pathLength; i++) {
            file >> path[i];
        }
        Table[{src, dst}] = path;
    }
}
```

#### 3. Multi-flow Communication
```cpp
// Send packets between multiple source-destination pairs
void SendPacket(int srcId, int dstId, std::vector<Ptr<PacketSink>>& sinks) {
    // Install PacketSink on destination
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", 
                               InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(dstNode);
    
    // Configure BulkSend application
    BulkSendHelper sourceHelper("ns3::TcpSocketFactory", 
                               InetSocketAddress(dstAddr, port));
    sourceHelper.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApp = sourceHelper.Install(srcNode);
}
```

## Implementation Details

### Core Files

#### Main Simulation File
- **`leo-lab6.cc`**: Complete LEO simulation with nix-vector routing
- **Network Configuration**: 36 satellites, 6 ground stations
- **Multi-flow Support**: Task 2 (single flow) and Task 3 (multiple flows)
- **Performance Monitoring**: Packet tracing and throughput measurement

#### Custom Routing Module (`nix-vector-routing/`)
- **`nix-vector-routing.h/cc`**: Core routing protocol implementation
- **`nix-vector-helper.h/cc`**: Helper class for routing setup
- **Path Management**: Pre-computed routing table management
- **Protocol Integration**: ns-3 routing protocol interface

#### Input/Output Files
- **`paths1.in`**: Pre-computed routing paths (6-hop routes)
- **`paths2.in`**: Alternative routing paths (4-6 hop routes)
- **`task2.out`**: Single flow packet tracing results
- **`task3.path1.out`**: Multi-flow throughput results

### Key Algorithms

#### 1. Path-based Routing
```cpp
// Route packets using pre-computed paths
Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, 
                          Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) {
    // Look up pre-computed path
    auto it = Table.find({sourceId, destId});
    if (it != Table.end()) {
        std::vector<int> path = it->second;
        // Use nix-vector for efficient routing
        Ptr<NixVector> nixVector = Create<NixVector>();
        // Build nix-vector from path
        return CreateRoute(nixVector, destAddr);
    }
    return nullptr;
}
```

#### 2. Packet Tracing
```cpp
// Trace TCP packets with data payload
static void EchoMacTxRx(std::string context, const Ptr<const Packet> packet) {
    Ptr<Packet> pCopy = packet->Copy();
    TcpHeader tcpHeader;
    
    if (pCopy->PeekHeader(tcpHeader)) {
        pCopy->RemoveHeader(tcpHeader);
        // Only trace packets with data payload (570 bytes)
        if (pCopy->GetSize() == 570) {
            std::string nodeIdStr = context.substr(context.find("NodeList/") + 9);
            nodeIdStr = nodeIdStr.substr(0, nodeIdStr.find("/"));
            uint32_t nodeId = std::stoi(nodeIdStr);
            
            Time now = Simulator::Now();
            std::string eventType = (context.find("MacTx") != std::string::npos) ? "MacTx" : "MacRx";
            
            std::cout << eventType << " at node: " << nodeId << ", now: " << now << std::endl;
        }
    }
}
```

#### 3. Throughput Measurement
```cpp
// Calculate throughput for multiple flows
if (Task == 3) {
    double totalBytes = 0;
    
    // Throughput for 36 -> 38
    double throughput1 = sinks[0]->GetTotalRx();
    std::cout << "36->38: " << throughput1 << std::endl;
    totalBytes += throughput1;
    
    // Throughput for 37 -> 40
    double throughput2 = sinks[1]->GetTotalRx();
    std::cout << "37->40: " << throughput2 << std::endl;
    totalBytes += throughput2;
    
    // Throughput for 39 -> 41
    double throughput3 = sinks[2]->GetTotalRx();
    std::cout << "39->41: " << throughput3 << std::endl;
    totalBytes += throughput3;
    
    std::cout << "Total throughput: " << totalBytes << std::endl;
}
```

## Lab Tasks

### Task 1: Nix-Vector Routing Implementation
- Implement custom nix-vector routing protocol
- Develop path pre-computation and storage system
- Integrate with ns-3 routing framework

### Task 2: Single Flow Analysis
- Implement single source-destination communication
- Add packet tracing for route analysis
- Measure end-to-end performance

### Task 3: Multi-flow Performance
- Implement multiple concurrent flows
- Measure individual and total throughput
- Analyze routing efficiency

### Task 4: Path Optimization
- Compare different routing strategies
- Analyze path length vs performance
- Optimize routing for satellite networks

## Performance Analysis

### Single Flow Results (Task 2)
- **Route**: 36 → 0 → 5 → 35 → 3 → 38 (6 hops)
- **Packet Tracing**: Complete MacTx/MacRx sequence
- **Performance**: Successful end-to-end delivery

### Multi-flow Results (Task 3)
- **Flow 1 (36→38)**: 38,400 bytes throughput
- **Flow 2 (37→40)**: 37,376 bytes throughput  
- **Flow 3 (39→41)**: 10,752 bytes throughput
- **Total Throughput**: 86,528 bytes

### Path Comparison
- **Paths1**: 6-hop routes with longer paths
- **Paths2**: 4-6 hop routes with optimized paths
- **Performance**: Shorter paths generally provide better throughput

## Technical Achievements

### Custom Routing Protocol
- **Nix-Vector Implementation**: Efficient routing for satellite networks
- **Path Pre-computation**: Fast packet forwarding
- **Protocol Integration**: Seamless ns-3 integration

### Network Simulation
- **LEO Integration**: Realistic satellite constellation simulation
- **Multi-flow Support**: Concurrent communication handling
- **Performance Monitoring**: Comprehensive packet tracing

### Algorithm Optimization
- **Path Selection**: Optimized routing strategies
- **Resource Management**: Efficient satellite resource utilization
- **Scalability**: Support for large satellite constellations

## Usage Instructions

### Prerequisites
- **ns-3**: Version 3.30+ with LEO module
- **C++ Compiler**: GCC 7.0+ or compatible
- **Custom Routing Module**: nix-vector-routing implementation

### Building the Project
```bash
# Navigate to lab directory
cd lab6-HelloHe110

# Build ns-3 with custom routing module
./waf configure --enable-examples
./waf build
```

### Running the Simulation
```bash
# Task 2: Single flow with packet tracing
./waf --run "leo-lab6 --Task=2 --in=paths1.in --out=task2.out"

# Task 3: Multi-flow throughput analysis
./waf --run "leo-lab6 --Task=3 --in=paths1.in --out=task3.path1.out"

# Compare different routing strategies
./waf --run "leo-lab6 --Task=3 --in=paths2.in --out=task3.path2.out"
```

### Configuration Options
```bash
# Custom simulation parameters
./waf --run "leo-lab6 \
  --Task=3 \
  --in=paths1.in \
  --out=results.out \
  --duration=100"
```

## Advanced Features

### Nix-Vector Routing
- **Efficient Forwarding**: Pre-computed paths for fast routing
- **Memory Optimization**: Compact path representation
- **Dynamic Adaptation**: Support for topology changes

### Multi-flow Management
- **Concurrent Flows**: Simultaneous multiple communications
- **Resource Sharing**: Efficient satellite resource allocation
- **Performance Isolation**: Independent flow performance

### Network Analysis
- **Packet Tracing**: Complete packet flow analysis
- **Throughput Measurement**: Real-time performance monitoring
- **Path Optimization**: Route selection and optimization

## Research Applications

### Satellite Network Routing
- **Dynamic Topologies**: Routing in changing satellite networks
- **Path Optimization**: Efficient route selection algorithms
- **Load Balancing**: Traffic distribution strategies

### Protocol Development
- **Custom Routing**: Novel routing protocol design
- **Performance Analysis**: Protocol evaluation and optimization
- **Network Simulation**: Large-scale network testing

## Future Enhancements

- **Adaptive Routing**: Dynamic path selection based on network conditions
- **Machine Learning**: AI-based routing optimization
- **Multi-objective**: Consider multiple routing criteria
- **Real-time Updates**: Dynamic path recomputation

## Acknowledgments

This lab builds upon the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and extends the ns-3 LEO module with custom routing capabilities.

Special thanks to:
- The NYCU Network Capstone course instructors
- The ns-3 development community
- The LEO satellite communication research community

## References

1. ns-3 Network Simulator Documentation
2. Nix-Vector Routing Algorithms
3. Satellite Communication Systems
4. Network Protocol Design

---

*This lab demonstrates advanced concepts in custom routing protocol development, satellite network simulation, and performance analysis. All implementations are original work developed as part of the Network Capstone course requirements.*