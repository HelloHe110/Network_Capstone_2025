# Lab 2: LEO Satellite Network Simulation - ns-3-LEO Implementation

## Overview

This lab implements a comprehensive Low Earth Orbit (LEO) satellite communication system using the ns-3 network simulator with the LEO module. The project focuses on satellite constellation simulation, inter-satellite links (ISL), ground station communication, and realistic propagation modeling for space-based networks.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 2 - LEO Satellite Network Simulation
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Satellite Constellation Modeling**: Implement realistic LEO satellite orbits and mobility
2. **Inter-Satellite Links (ISL)**: Establish communication links between satellites
3. **Ground-Satellite Communication**: Model user terminal and gateway connections
4. **Propagation Loss Modeling**: Implement realistic space-ground propagation models
5. **Network Performance Analysis**: Evaluate delay, throughput, and routing performance
6. **Multi-Constellation Support**: Support for Starlink, Telesat, and custom constellations

## Technical Architecture

### System Components

#### Core Simulation Framework
- **ns-3 Network Simulator**: Base simulation platform
- **LEO Module**: Custom satellite communication module
- **AODV Routing**: Ad-hoc On-Demand Distance Vector routing protocol
- **TCP/UDP Applications**: Traffic generation and analysis

#### Satellite Constellation Support
- **Starlink Constellation**: SpaceX's LEO satellite network
- **Telesat Constellation**: Telesat's LEO satellite system
- **Custom Orbits**: Configurable orbital parameters
- **Real-time Mobility**: Dynamic satellite position updates

### Key Features

#### 1. Satellite Orbit Modeling
```cpp
// Orbit definition with altitude, inclination, planes, and satellites
LeoOrbit orbit(1200, 20, 32, 16);  // 1200km altitude, 20° inclination, 32 planes, 16 sats/plane

// Install satellites from CSV file or programmatic definition
NodeContainer satellites = orbit.Install("starlink.csv");
```

#### 2. Ground Station Management
```cpp
// Install ground stations in grid pattern
LeoGndNodeHelper ground;
NodeContainer stations = ground.Install(20, 20);  // 20x20 grid

// Install specific user terminals
LeoLatLong source(51.399, 10.536);      // Middle Europe
LeoLatLong destination(40.76, -73.96);  // East Coast USA
NodeContainer users = ground.Install(source, destination);
```

#### 3. Channel Configuration
```cpp
// User terminal to satellite channels
LeoChannelHelper utCh;
utCh.SetConstellation("StarlinkUser");
NetDeviceContainer utNet = utCh.Install(satellites, stations);

// Inter-satellite links
IslHelper islCh;
islCh.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
NetDeviceContainer islNet = islCh.Install(satellites);
```

## Implementation Details

### Core Files Structure

#### Main Simulation Files
- **`bf.m`**: MATLAB beamforming analysis for satellite-ground links
- **`nobf.m`**: Non-beamforming reference implementation
- **`wscript`**: Build configuration for ns-3

#### Essential Wave Analysis (`ewa_function/`)
- **`uniform.m`**: Uniform array beamforming
- **`dtft.m`**: Discrete-Time Fourier Transform
- **`friis_equation.m`**: Free-space path loss calculation
- **`steer.m`**: Antenna array steering
- **`scan.m`**: Array scanning functions
- **`bwidth.m`**: Beamwidth calculations

#### LEO Module Components (`model/`)
- **`leo-orbit.h/cc`**: Orbit definition and management
- **`leo-circular-orbit-mobility-model.h/cc`**: Satellite mobility simulation
- **`leo-propagation-loss-model.h/cc`**: Space-ground propagation modeling
- **`leo-mock-channel.h/cc`**: Satellite communication channels
- **`leo-mock-net-device.h/cc`**: Satellite network devices

#### Helper Classes (`helper/`)
- **`leo-helper.h/cc`**: Main LEO network configuration
- **`leo-orbit-node-helper.h/cc`**: Satellite node management
- **`leo-channel-helper.h/cc`**: Channel configuration
- **`isl-helper.h/cc`**: Inter-satellite link management
- **`ground-node-helper.h/cc`**: Ground station management

#### Example Applications (`examples/`)
- **`leo-delay-tracing-example.cc`**: Delay measurement simulation
- **`leo-bulk-send-example.cc`**: Throughput analysis simulation
- **`leo-circular-orbit-tracing-example.cc`**: Orbit tracking simulation

### Key Algorithms

#### 1. Satellite Orbit Calculation
```cpp
// Circular orbit mobility model
class LeoCircularOrbitMobilityModel : public MobilityModel
{
    // Calculate satellite position based on orbital parameters
    Vector GetPosition() const;
    // Update position based on simulation time
    void UpdatePosition();
};
```

#### 2. Propagation Loss Modeling
```cpp
// Free-space path loss for space-ground links
double LeoPropagationLossModel::DoCalcRxPower(double txPowerDbm,
                                             Ptr<MobilityModel> a,
                                             Ptr<MobilityModel> b) const
{
    // Calculate distance and apply Friis equation
    double distance = a->GetDistanceFrom(b);
    return txPowerDbm - 20*log10(4*M_PI*distance/lambda);
}
```

#### 3. Inter-Satellite Link Management
```cpp
// ISL channel with dynamic topology
class IslMockChannel : public Channel
{
    // Maintain active links between satellites
    void UpdateLinks();
    // Handle link establishment/teardown
    bool IsLinkActive(Ptr<Node> a, Ptr<Node> b);
};
```

## Lab Tasks

### Task 1: Distance and Elevation Calculation
- Calculate 3D distances between satellites and ground stations
- Compute elevation angles for optimal beam steering
- Implement coordinate system transformations

### Task 2: Path Loss Analysis
- Implement Friis equation for space-ground propagation
- Calculate received power and signal-to-noise ratios
- Analyze path loss variations with satellite movement

### Task 3: Network Performance Evaluation
- Measure end-to-end delay through satellite network
- Analyze throughput with different traffic patterns
- Evaluate routing performance with dynamic topology

### Task 4: Constellation Comparison
- Compare Starlink vs Telesat constellation performance
- Analyze coverage patterns and connectivity
- Evaluate handover frequency and impact

## Simulation Capabilities

### Traffic Generation
- **UDP Echo**: Delay measurement applications
- **TCP Bulk Transfer**: Throughput analysis
- **Custom Applications**: Configurable traffic patterns

### Network Protocols
- **AODV Routing**: Dynamic route discovery and maintenance
- **IPv4/IPv6**: Internet protocol support
- **TCP/UDP**: Transport layer protocols

### Monitoring and Analysis
- **Packet Tracing**: Complete packet flow analysis
- **Mobility Tracing**: Satellite position tracking
- **Performance Metrics**: Delay, throughput, packet loss

## Usage Instructions

### Prerequisites
- **ns-3**: Version 3.30 or later
- **C++ Compiler**: GCC 7.0+ or compatible
- **MATLAB**: For beamforming analysis
- **Python**: For data analysis scripts

### Building the Project
```bash
# Navigate to lab directory
cd lab2-HelloHe110

# Configure and build ns-3 with LEO module
./waf configure --enable-examples
./waf build

# Run example simulations
./waf --run "leo-delay --orbitFile=data/orbits/starlink.csv --duration=1000"
./waf --run "leo-bulk-send --orbitFile=data/orbits/telesat.csv --duration=1000"
```

### Running MATLAB Analysis
```matlab
% Navigate to lab directory
cd lab2-HelloHe110

% Run beamforming analysis
run('bf.m')

% Run non-beamforming analysis
run('nobf.m')
```

### Example Simulations
```bash
# Delay measurement with Starlink constellation
./waf --run "leo-delay \
  --orbitFile=data/orbits/starlink.csv \
  --constellation=StarlinkGateway \
  --islRate=2Gbps \
  --islEnabled=true \
  --duration=1000"

# Throughput analysis with Telesat constellation
./waf --run "leo-bulk-send \
  --orbitFile=data/orbits/telesat.csv \
  --constellation=TelesatGateway \
  --islRate=2Gbps \
  --islEnabled=true \
  --duration=1000"
```

## Data Files

### Orbit Data (`data/orbits/`)
- **`starlink.csv`**: Starlink constellation parameters
- **`telesat.csv`**: Telesat constellation parameters

### Test Data (`data/test/`)
- **`ground-stations.txt`**: Ground station coordinates
- **`waypoints.txt`**: Test waypoint locations

### Configuration Files
- **`xyz_positions.txt`**: 3D position coordinates
- **`pathloss.txt`**: Calculated path loss values

## Results and Outputs

### Generated Files
- **`result.pdf`**: Simulation results and analysis
- **`report.pdf`**: Comprehensive lab report
- **Trace Files**: Network performance data

### Performance Metrics
- **End-to-End Delay**: Packet transmission delays
- **Throughput**: Data transfer rates
- **Packet Loss**: Network reliability metrics
- **Coverage Analysis**: Satellite visibility patterns

## Technical Achievements

### Satellite Communication
- **Realistic Orbit Modeling**: Accurate satellite position calculation
- **Dynamic Topology**: Real-time link establishment/teardown
- **Multi-Constellation Support**: Starlink, Telesat, and custom orbits

### Network Simulation
- **ns-3 Integration**: Full network simulator compatibility
- **Protocol Stack**: Complete TCP/IP implementation
- **Routing Protocols**: AODV for dynamic networks

### Performance Analysis
- **Comprehensive Tracing**: Complete network monitoring
- **Statistical Analysis**: Performance evaluation tools
- **Visualization**: Network topology and performance plots

## Advanced Features

### Constellation Management
- **Orbital Mechanics**: Realistic satellite motion
- **Handover Simulation**: Satellite-to-satellite transitions
- **Coverage Analysis**: Global coverage patterns

### Network Optimization
- **Link Selection**: Optimal satellite selection
- **Load Balancing**: Traffic distribution strategies
- **QoS Management**: Quality of service implementation

### Research Applications
- **Protocol Development**: New routing algorithms
- **Performance Evaluation**: Network optimization studies
- **Scalability Analysis**: Large-scale constellation simulation

## Future Enhancements

- **Machine Learning**: AI-based satellite selection
- **5G Integration**: 5G-NR satellite communication
- **Edge Computing**: Satellite edge processing
- **Real-time Simulation**: Hardware-in-the-loop testing

## Acknowledgments

This lab builds upon the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and utilizes the ns-3-LEO module originally developed by Tim Schubert and the research community.

Special thanks to:
- The NYCU Network Capstone course instructors
- The ns-3 development community
- The LEO satellite communication research community
- The ns-3-LEO module developers

## References

1. Schubert, T. "ns-3-leo: Evaluation Tool for Satellite Swarm Communication Protocols" - IEEE Access
2. ns-3 Network Simulator Documentation
3. AODV Routing Protocol Specification (RFC 3561)
4. Satellite Communication Systems Engineering Principles

---

*This lab demonstrates advanced concepts in satellite communication, network simulation, and space-based networking. All implementations are original work developed as part of the Network Capstone course requirements.*
