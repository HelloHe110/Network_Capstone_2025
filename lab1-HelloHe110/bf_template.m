%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DO NOT MODIFY THIS SECTION. Any changes will result in a penalty.
close all; clear; clc;
addpath ./ewa_function;
rng(77);

% Environment Configurations
freq = 24e9;
tx_node_number = 1;      % Number of Tx users
tx_antenna_number = 16;  % Number of Tx antennas
rx_node_number = 2;      % Number of Rx users
rx_antenna_number = 1;   % Number of Rx antennas
tx_beam_direction = 0:10:180; % degree
d = 0.5;                 % Distance between antennas (multiple of wavelength)
P_tx_dBm = 20;           % Transmission power of Tx (dBm)
N0_dBm = -88;            % Noise power (dBm)
tx_location = [0, 0];    % Tx location

% Random Rx location
rx_location = zeros(rx_node_number, 2);
for i = 1:rx_node_number
    r = 5 + 20 * rand();    % Random distance between 5 and 25 meters (m)
    angle = 180 * rand();
    x = r * cosd(angle);    % Beam direction with a small random offset
    y = r * sind(angle);    % Beam direction with a small random offset
    rx_location(i, :) = [x, y];
end

fprintf('Rx1 location: (%.2f, %.2f)\n', rx_location(1,1), rx_location(1,2));
fprintf('Rx2 location: (%.2f, %.2f)\n\n', rx_location(2,1), rx_location(2,2));

resolution = 360;           % Number of angles dividing 180 degrees
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fprintf('Task 1: Calculate AoD\n');
fprintf('Task 2: Calculate Rx power and SNR\n');
fprintf('Task 3: Calculate SINR of two concurrent beams\n');
fprintf('Report\n');