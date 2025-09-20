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


fprintf('Task 1: Distance calculation\n');
% ===========================  START of Task 1: Calculate AoD ==============

    % 3. Calculate horizontal distance between Tx and Rx using Pythagorean theorem
    horizontal_dist = sqrt((rx_location(:,1) - tx_location(1)).^2 + (rx_location(:,2) - tx_location(2)).^2);
    total_dist = sqrt((rx_location(:,1) - tx_location(1)).^2 + (rx_location(:,2) - tx_location(2)).^2 + (rx_location(:,3) - tx_location(3)).^2);
    % total_dist = 1200009.000003;
    dprintf(sprintf('Rx1 horizontal distance: %.6f m\n', horizontal_dist(1)));
    printf(sprintf('Rx1 total distance: %.6f m\n', total_dist));
    % 4. Calculate elevation angle between Tx and Rx
    %    - Uses arctangent of height difference over horizontal distance
    %    - Gives vertical angle from horizontal plane
    elevation = atan2d(horizontal_dist, rx_location(:,3) - tx_location(3)); % Calculate elevation angle
    if elevation >= 90
        elevation = 180 - elevation;
    end
    % elevation = atan2d(rx_location(:,3) - tx_location(3), horizontal_dist); % Calculate elevation angle
    [~, tx_idx] = min(abs(tx_beam_direction - elevation));
    printf(sprintf('Rx1 elevation angle: %.6f degree\n', elevation(1)));
    printf(sprintf('Actual tx beam angle: %.6f\n', tx_beam_direction(tx_idx)));
% ===========================  END of Task 1: Calculate AoD ================

fprintf('Task2: pathloss\n');
% ===========================  START of Task 2: Calculate Rx power and SNR ==============
    
    c = 299792458; % Speed of light (m/s)
    lambda = c / freq; % Wavelength (m)
    phi = (1 : resolution) * pi / resolution; % Angle in radians
    phi_d = phi * 180 / pi; % Angle in degrees
    psi = 2 * pi * d * sin(phi); % Phase shift
% ==========================================================================================

    % beam_resolutions = (0.25:(90/resolution):90); % Beam resolution
    beam_resolutions = (0.5:(180/resolution):90); % Beam resolution
    [~, AoD1_resolutions_idx] = min(abs(beam_resolutions - elevation)); % AoD1 resolution
    optimal_beam_resolutions_idx = AoD1_resolutions_idx; % Optimal beam resolution index
    optimal_beam_resolutions = [beam_resolutions(AoD1_resolutions_idx)]; % Optimal beam resolution

    % Hint: Calculate the Tx Gain
    Gains = zeros(rx_node_number, resolution);
    Gain = zeros(1, resolution);

    G_tx = zeros(rx_node_number, 1);
    SNR_dB = zeros(rx_node_number, 1);

    rx_Dists = zeros(rx_node_number, 1);
    rx_Beam_idxs = zeros(rx_node_number, 1);
    
    for i = 1:rx_node_number
        % dprintf(sprintf('Rx%d optimal beam resolution: %.6f degrees\n', i, optimal_beam_resolutions(i)));

        [weights, hpdw] = uniform(d, tx_beam_direction(tx_idx), tx_antenna_number); % 計算權重
        Gain = abs(dtft(weights, -psi)).^2; % 計算 Gain
        Gains(i, :) = Gain; % 儲存 Gain for report use
        
        rx_Dists(i) = horizontal_dist(i);
        [~, rx_Beam_idxs(i)] = max(Gain(:, optimal_beam_resolutions_idx(i))); % 計算 Rx 所在的 beam index

        G_tx(i) = Gain(rx_Beam_idxs(i), optimal_beam_resolutions_idx(i)); % 計算 Tx Gain

        SNR_dB(i) = P_tx_dBm + friis_equation(freq, G_tx(i), 1, total_dist(i)) - N0_dBm; % 計算 SNR
    end

    % Hint: Calculate the Rx power and SNR
    dprintf(sprintf('Tx Gain: %.6f dBm\n', G_tx(1)));
    pathloss = -friis_equation(freq, G_tx(1), 1, total_dist(1));
    printf(sprintf('Pathloss: %.6f dB\n', pathloss));
    printf(sprintf('Rx1 power: %.6f dBm\n', P_tx_dBm + friis_equation(freq, G_tx(1), 1, total_dist(1))));
    % printf(sprintf('Rx1 power: %.6f dBm\n', P_tx_dBm - pathloss));
    % printf(sprintf('Rx1 SNR: %f dB\n', SNR_dB(1)));

    fileID = fopen('pathloss.txt', 'w');
    fprintf(fileID, '%f', pathloss);
    fclose(fileID);

% ===========================  END of Task 2: Calculate Rx power and SNR ================

% fprintf('Task 3: Calculate SINR of two concurrent beams\n');
% ===========================  START of Task 3: Calculate SINR of two concurrent beams ==============
    % Hint: Calculate the Interference Power
    % !! User2's Rx power == User1's Interference power !!
    % printf(sprintf('Rx1 interference power: %.6f dBm\n', P_tx_dBm + friis_equation(freq, G_interference(2), 1, rx_Dists(1))));
    % Hint: Calculate the SINR
    % printf(sprintf('Rx1 SINR: %.6f dB\n', (P_tx_dBm + friis_equation(freq, G_tx(1), 1, rx_Dists(1))) - Wat2dBm(dBm2Wat(P_tx_dBm + friis_equation(freq, G_interference(2), 1, rx_Dists(1))) + dBm2Wat(N0_dBm))));
    % user1 的 SINR 是 beam1 到 user1 的 Prx 除以 (beam2 到 user1 的 interference power + noise power) - by TA
% ===========================  END of Task 3: Calculate SINR of two concurrent beams ================

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

% fprintf('Report\n');
% % ===========================  START of Report ================================================
    % Figure 1: Cartesian plot for user 1(x axis: angle in degree, y axis: Tx gain)
    % dprintf(sprintf('Cartesian plot for user 1\n'));
    % plot_tx_gain(Gains(1, :), phi_d, 1);
%     % Figure 2: Cartesian plot for user 2(x axis: angle in degree, y axis: Tx gain)
%     dprintf(sprintf('Cartesian plot for user 2\n'));
%     plot_tx_gain(Gains(2, :), phi_d, 2);
    % Figure 3: Polar plot corresponding to figure 1
    % dprintf(sprintf('Polar plot corresponding to figure 1\n'));
    % plot_tx_gain_polar(Gains(1, :), phi, 1);

%     % Avg task
%     dprintf(sprintf('Avg from 20 random run!\n'));
%     avg_20_random_run();



% ===========================  END of Report ================================================

    % ================== functions for supporting report ==================
    function plot_tx_gain(Gain, phi_d, user_id)
        % plot_tx_gain - Plot Tx Gain vs Angle
        % Args:
        %   Gain: Tx gain values (vector)
        %   phi_d: Corresponding angles in degrees (vector)
        %   user_id: User identifier for the plot title

        figure;
        plot(phi_d, Gain, 'b'); % 繪製 Gain vs Angle
        xlabel('Angle (degree)');
        ylabel('Gain (dB)');
        title(sprintf('User %d Tx Gain', user_id));
        grid on; % Adding a grid for better visualization
    end
    function plot_tx_gain_polar(Gain, phi, user_id)
        % plot_tx_gain_polar - Plot Tx Gain using a polar plot
        % Args:
        %   Gain: Tx gain values (vector)
        %   phi: Corresponding angles in radians (vector)
        %   user_id: User identifier for the plot title

        figure;
        polarplot(phi, Gain, 'b'); % 繪製 Polar Plot
        title(sprintf('User %d Tx Gain (Polar Plot)', user_id));
        grid on; % Adding a grid for better visualization
    end
    % ============== END of functions for supporting report ================