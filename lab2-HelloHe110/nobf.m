%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DO NOT MODIFY THIS SECTION. Any changes will result in a penalty.
close all; clear; clc;
addpath ./ewa_function;
rng(77);

% Environment Configurations
freq = 24e9;
tx_node_number = 1;      % Number of Tx users
tx_antenna_number = 16;  % Number of Tx antennas
rx_node_number = 1;      % Number of Rx users
rx_antenna_number = 1;   % Number of Rx antennas
tx_beam_direction = 0:5:90; % degree
d = 0.5;                 % Distance between antennas (multiple of wavelength)
P_tx_dBm = 20;           % Transmission power of Tx (dBm)
N0_dBm = -88;            % Noise power (dBm)
% Read locations from file
locations = dlmread('xyz_positions.txt');
tx_location = locations(1, 1:3);    % Take x,y from first line for Tx location

% Set Rx locations from file
rx_location = zeros(rx_node_number, 3);
for i = 1:rx_node_number
    rx_location(i, :) = locations(i+1, 1:3);  % Take x,y from subsequent lines for Rx locations
end

fprintf('Tx location: (%.2f, %.2f, %.2f)\n', tx_location(1), tx_location(2), tx_location(3));
fprintf('Rx1 location: (%.2f, %.2f, %.2f)\n', rx_location(1,1), rx_location(1,2), rx_location(1,3));
% fprintf('Rx2 location: (%.2f, %.2f)\n\n', rx_location(2,1), rx_location(2,2));
% return;
resolution = 360;           % Number of angles dividing 180 degrees
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% pathloss calculation
fprintf('Task 1: Distance calculation\n');
    horizontal_dist = sqrt((rx_location(:,1) - tx_location(1)).^2 + (rx_location(:,2) - tx_location(2)).^2);
    total_dist = sqrt((rx_location(:,1) - tx_location(1)).^2 + (rx_location(:,2) - tx_location(2)).^2 + (rx_location(:,3) - tx_location(3)).^2);
    dprintf(sprintf('Rx1 horizontal distance: %.6f m\n', horizontal_dist(1)));
    printf(sprintf('Rx1 total distance: %.6f m\n', total_dist));

fprintf('Task2: pathloss\n');
pathloss = -friis_equation(freq, 1, 1, total_dist(1));
printf(sprintf('Pathloss: %.6f dB\n', pathloss));
printf(sprintf('Rx1 power: %.6f dBm\n', P_tx_dBm - pathloss)); 
printf(sprintf('SNR: %.6f dB\n', P_tx_dBm - pathloss - N0_dBm));
    fileID = fopen('pathloss.txt', 'w');
    fprintf(fileID, '%f', pathloss);
    fclose(fileID);
% ========================================================================================================================================================
    function power_dBm = Wat2dBm(power_Wat)
        power_dBm = 10 * log10(power_Wat * 1000);
    end
    function power_Wat = dBm2Wat(power_dBm)
        power_Wat = 10^((power_dBm) / 10) * 10^-3;
    end
    function printf(str)
        % Print the input string with a tab character in the front
        fprintf('\t%s', str);
    end
    function dprintf(str)
        % Print the input debug string with a tab character in the front
        fprintf('\t// Debug - %s', str);
    end
% ======================================================================================================================================================== 