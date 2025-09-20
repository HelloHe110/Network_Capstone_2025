# Lab 1: Beamforming Fundamentals - Antenna Array Processing

## Overview

This lab implements fundamental beamforming algorithms for antenna array processing in wireless communication systems. The project focuses on uniform linear array (ULA) beamforming, direction of arrival (DOA) estimation, and signal-to-noise ratio (SNR) analysis using MATLAB.

## Student Information

- **Student ID**: 111550022
- **Lab**: Lab 1 - Beamforming Fundamentals
- **Course**: Network Capstone 2025, NYCU

## Lab Objectives

1. **Angle of Departure (AoD) Calculation**: Compute optimal beam directions for multiple receivers
2. **Beamforming Implementation**: Apply uniform linear array beamforming techniques
3. **Power and SNR Analysis**: Calculate received power and signal-to-noise ratios
4. **Interference Analysis**: Evaluate signal-to-interference-plus-noise ratio (SINR) for concurrent beams
5. **Performance Visualization**: Generate comprehensive plots and analysis reports

## Technical Specifications

### System Parameters
- **Frequency**: 24 GHz (millimeter wave)
- **Transmit Power**: 20 dBm
- **Noise Power**: -88 dBm
- **Antenna Spacing**: 0.5λ (half wavelength)
- **Array Configuration**: Uniform Linear Array (ULA)
- **Beam Codebook**: 0° to 180° in 10° increments

### Antenna Array Configuration
- **Transmitter**: 16-element uniform linear array
- **Receivers**: Single antenna elements
- **Beam Steering**: Electronic beamforming with phase control
- **Coverage**: 180° azimuth range

## Implementation Details

### Core Files

#### Main Implementation Files
- **`bf.m`**: Complete beamforming implementation with 2 receivers
- **`bf_modify.m`**: Modified version for 3D positioning with elevation angles
- **`bf_template.m`**: Template file provided by course instructors

#### Essential Wave Analysis Functions (`ewa_function/`)
- **`uniform.m`**: Uniform array weight calculation and beam steering
- **`dtft.m`**: Discrete-Time Fourier Transform for array pattern analysis
- **`friis_equation.m`**: Free-space path loss calculation
- **`steer.m`**: Array steering towards specified angles
- **`scan.m`**: Array scanning with progressive phase shifts
- **`bwidth.m`**: 3-dB beamwidth calculation

### Key Algorithms

#### 1. Angle of Departure (AoD) Calculation
```matlab
% Calculate horizontal AoD using arctangent
AoD = atan2d(rx_location(:,2) - tx_location(2), rx_location(:,1) - tx_location(1));
AoD = mod(AoD, 180); % Normalize to 0-180° range

% Find closest beam direction from codebook
[~, idx] = min(abs(tx_beam_direction - AoD));
optimal_beam_direction = tx_beam_direction(idx);
```

#### 2. Uniform Linear Array Beamforming
```matlab
% Calculate array weights and steering
[weights, hpdw] = uniform(d, optimal_beam_direction, tx_antenna_number);

% Compute array gain pattern using DTFT
Gain = abs(dtft(weights, -psi)).^2;
```

#### 3. Friis Path Loss Model
```matlab
% Calculate received power using Friis equation
receiving_power_dbm = 10*log10(tx_gain) + 10*log10(rx_gain) - 20*log10((4*pi*distance)/wavelength);
```

#### 4. SNR and SINR Calculation
```matlab
% Signal-to-Noise Ratio
SNR_dB = P_tx_dBm + friis_equation(freq, G_tx, 1, distance) - N0_dBm;

% Signal-to-Interference-plus-Noise Ratio
SINR_dB = signal_power - Wat2dBm(dBm2Wat(interference_power) + dBm2Wat(noise_power));
```

## Lab Tasks

### Task 1: AoD Calculation
- Calculate actual angles of departure for receivers
- Find optimal beam directions from predefined codebook
- Handle both 2D (azimuth only) and 3D (azimuth + elevation) scenarios

### Task 2: Power and SNR Analysis
- Implement uniform linear array beamforming
- Calculate transmitter gain using DTFT
- Compute received power using Friis equation
- Determine signal-to-noise ratio for each receiver

### Task 3: Interference Analysis
- Calculate interference power from concurrent beams
- Compute signal-to-interference-plus-noise ratio (SINR)
- Analyze beam interference patterns

### Task 4: Performance Analysis and Visualization
- Generate Cartesian plots of antenna gain patterns
- Create polar plots for directional analysis
- Compare performance with different antenna configurations
- Conduct statistical analysis over multiple random scenarios

## Key Features

### Advanced Beamforming
- **Electronic Steering**: Precise beam direction control
- **Pattern Synthesis**: Optimized antenna gain patterns
- **Interference Mitigation**: Analysis of concurrent beam interference

### 3D Positioning Support
- **Elevation Angle Calculation**: Vertical angle computation
- **3D Distance Calculation**: Euclidean distance in 3D space
- **Multi-dimensional Beamforming**: Support for both azimuth and elevation

### Comprehensive Analysis
- **Statistical Evaluation**: 20-run average performance analysis
- **Multiple Configurations**: Different antenna numbers and codebook sizes
- **Visualization Tools**: Cartesian and polar plot generation

## Results and Outputs

### Generated Files
- **`pathloss.txt`**: Calculated path loss values
- **`xyz_positions.txt`**: 3D position coordinates
- **`result.pdf`**: Comprehensive analysis report
- **`report.pdf`**: Detailed lab report

### Visualization Outputs
1. **Gain Pattern Plots**: Cartesian and polar representations
2. **Comparison Charts**: Different antenna configurations
3. **Statistical Analysis**: Performance metrics over multiple runs

## Usage Instructions

### Prerequisites
- MATLAB R2018b or later
- Signal Processing Toolbox
- Communications Toolbox (optional)

### Running the Lab
```matlab
% Navigate to lab directory
cd lab1-HelloHe110

% Run main beamforming simulation
run('bf.m')

% Run modified 3D version
run('bf_modify.m')
```

### Key Functions
```matlab
% Calculate uniform array weights
[weights, beamwidth] = uniform(spacing, angle, num_antennas);

% Compute array gain pattern
gain_pattern = abs(dtft(weights, phase_shift)).^2;

% Calculate path loss
pathloss = friis_equation(frequency, tx_gain, rx_gain, distance);
```

## Technical Achievements

### Signal Processing
- Implemented efficient DTFT-based pattern calculation
- Developed robust beam steering algorithms
- Created comprehensive SNR/SINR analysis tools

### Antenna Theory
- Applied uniform linear array theory
- Implemented electronic beamforming
- Analyzed 3-dB beamwidth characteristics

### Communication Systems
- Integrated Friis path loss model
- Developed interference analysis framework
- Created statistical performance evaluation tools

## Performance Metrics

### Beamforming Accuracy
- **Angular Resolution**: 10° codebook precision
- **Beam Steering**: Electronic steering with phase control
- **Pattern Quality**: Optimized gain patterns with minimal sidelobes

### Signal Quality Analysis
- **SNR Calculation**: Accurate signal-to-noise ratio computation
- **SINR Analysis**: Comprehensive interference evaluation
- **Path Loss Modeling**: Realistic propagation modeling

## Future Enhancements

- **Adaptive Beamforming**: Implementation of adaptive algorithms
- **Multi-user MIMO**: Extension to multiple-input multiple-output systems
- **Machine Learning**: AI-based beam selection optimization
- **Real-time Processing**: Hardware implementation considerations

## Acknowledgments

This lab builds upon the excellent work from the [NYCU-NETCAP2025](https://github.com/NYCU-NETCAP2025) organization and utilizes the Essential Wave Analysis (EWA) library developed by Sophocles J. Orfanidis.

Special thanks to:
- The NYCU Network Capstone course instructors
- The EWA library developers
- The antenna array research community

## References

1. Orfanidis, S.J. "Electromagnetic Waves and Antennas" - EWA Library
2. Balanis, C.A. "Antenna Theory: Analysis and Design"
3. Mailloux, R.J. "Phased Array Antenna Handbook"
4. Friis, H.T. "A Note on a Simple Transmission Formula"

---

*This lab demonstrates fundamental concepts in antenna array processing, beamforming algorithms, and wireless communication system analysis. All implementations are original work developed as part of the Network Capstone course requirements.*