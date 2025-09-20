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


fprintf('Task 1: Calculate AoD\n');
% ===========================  START of Task 1: Calculate AoD ==============
    % % This section calculates the angles and distances between transmitter and receiver:
    % % 1. Calculate Angle of Departure (AoD) in the horizontal plane using arctangent
    % %    - Takes difference in x,y coordinates between Rx and Tx
    % %    - Returns angle in degrees between -180 to 180
    % AoD = atan2d(rx_location(:,2) - tx_location(2), rx_location(:,1) - tx_location(1)); % Calculate horizontal AoD
    % printf(sprintf('Rx1 actual Angle: %.6f degree\n', AoD(1)));
    % AoD = mod(AoD, 180); % Normalize angle to 0-180 degree range

    % % 2. Find the closest beam direction from the predefined codebook
    % %    - Compares AoD with available beam directions
    % %    - Selects direction with minimum angular difference
    % [~, idx1] = min(abs(tx_beam_direction - AoD(1)));
    % optimal_beam_direction = [tx_beam_direction(idx1)];
    % printf(sprintf('User 1 AoD: %.2f degrees, Closest Beam: %.2f degrees\n', AoD(1), optimal_beam_direction(1)));

    % 3. Calculate horizontal distance between Tx and Rx using Pythagorean theorem
    horizontal_dist = sqrt((rx_location(:,1) - tx_location(1)).^2 + (rx_location(:,2) - tx_location(2)).^2);
    printf(sprintf('Rx1 horizontal distance: %.6f m\n', horizontal_dist(1)));
    
    % 4. Calculate elevation angle between Tx and Rx
    %    - Uses arctangent of height difference over horizontal distance
    %    - Gives vertical angle from horizontal plane
    elevation = atan2d(rx_location(:,3) - tx_location(3), horizontal_dist); % Calculate elevation angle
    printf(sprintf('Rx1 elevation angle: %.6f degree\n', elevation(1)));
% ===========================  END of Task 1: Calculate AoD ================

fprintf('Task 2: Calculate Rx power and SNR\n');
% ===========================  START of Task 2: Calculate Rx power and SNR ==============
    
    c = 299792458; % Speed of light (m/s)
    lambda = c / freq; % Wavelength (m)
    phi = (1 : resolution) * pi / resolution; % Angle in radians
    phi_d = phi * 180 / pi; % Angle in degrees
    psi = 2 * pi * d * sin(phi); % Phase shift
% ==========================================================================================

    beam_resolutions = (0.5:(180/resolution):180); % Beam resolution
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
        dprintf(sprintf('Rx%d optimal beam resolution: %.6f degrees\n', i, optimal_beam_resolutions(i)));

        [weights, hpdw] = uniform(d, elevation(i), tx_antenna_number); % 計算權重
        Gain = abs(dtft(weights, -psi)).^2; % 計算 Gain
        Gains(i, :) = Gain; % 儲存 Gain for report use
        
        rx_Dists(i) = horizontal_dist(i);
        [~, rx_Beam_idxs(i)] = max(Gain(:, optimal_beam_resolutions_idx(i))); % 計算 Rx 所在的 beam index

        G_tx(i) = Gain(rx_Beam_idxs(i), optimal_beam_resolutions_idx(i)); % 計算 Tx Gain

        SNR_dB(i) = P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i)) - N0_dBm; % 計算 SNR
    end

    % Hint: Calculate the Rx power and SNR
    dprintf(sprintf('Tx Gain: %.6f dBm\n', G_tx(1)));
    pathloss = -friis_equation(freq, G_tx(1), 1, rx_Dists(1));
    printf(sprintf('Pathloss: %.6f dB\n', pathloss));
    printf(sprintf('Rx1 power: %.6f dBm\n', P_tx_dBm + friis_equation(freq, G_tx(1), 1, rx_Dists(1))));
    printf(sprintf('Rx1 SNR: %.6f dB\n', SNR_dB(1)));

    fileID = fopen('pathloss.txt', 'w');
    fprintf(fileID, '%f', pathloss);
    fclose(fileID);

% ===========================  END of Task 2: Calculate Rx power and SNR ================

fprintf('Task 3: Calculate SINR of two concurrent beams\n');
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
    dprintf(sprintf('Cartesian plot for user 1\n'));
    plot_tx_gain(Gains(1, :), phi_d, 1);
%     % Figure 2: Cartesian plot for user 2(x axis: angle in degree, y axis: Tx gain)
%     dprintf(sprintf('Cartesian plot for user 2\n'));
%     plot_tx_gain(Gains(2, :), phi_d, 2);
    % Figure 3: Polar plot corresponding to figure 1
    dprintf(sprintf('Polar plot corresponding to figure 1\n'));
    plot_tx_gain_polar(Gains(1, :), phi, 1);
%     % Figure 4: Polar plot corresponding to figure 2
%     dprintf(sprintf('Polar plot corresponding to figure 2\n'));
%     plot_tx_gain_polar(Gains(2, :), phi, 2);

%     % Figure 5~8: Comparison of Tx Gain for different antenna numbers and beam codebooks
%     dprintf(sprintf('Comparisons\n'));
%     plot_optimal_beam(rx_location);

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
    function plot_optimal_beam(rx_location)
        % Environment Configurations
        freq = 24e9;
        rx_node_number = 2;
        d = 0.5;
        P_tx_dBm = 20;
        N0_dBm = -88;
        tx_location = [0, 0];

        resolution = 360;
        % beam_codebooks = {[0:10:180], [0:10:180]};
        % antenna_numbers = [16, 16];
        beam_codebooks = {[0:5:180], [0:15:180]};
        antenna_numbers = [4, 8];

        % Compute optimal beam direction
        AoD = atan2d(rx_location(:,2) - tx_location(2), rx_location(:,1) - tx_location(1));
        AoD = mod(AoD, 180);

        % Initialize gain storage
        Gains = cell(2, 2);
        phi = (1 : resolution) * pi / resolution;
        phi_d = phi * 180 / pi;
        psi = 2 * pi * d * cos(phi);

        for a_idx = 1:length(antenna_numbers)
            tx_antenna_number = antenna_numbers(a_idx);
            printf(sprintf('--- Tx Antennas: %d ---\n\n', tx_antenna_number));
            for cb_idx = 1:length(beam_codebooks)
                if (cb_idx == 1)
                    printf(sprintf('--- Codebook Size:  5 ---\n\n'));
                else
                    printf(sprintf('--- Codebook Size: 15 ---\n\n'));
                end
                
                codebook = beam_codebooks{cb_idx};
                [~, idx1] = min(abs(codebook - AoD(1)));
                [~, idx2] = min(abs(codebook - AoD(2)));
                optimal_beam_direction = [codebook(idx1), codebook(idx2)];

                rx_Dists = [norm(rx_location(1, :) - tx_location), norm(rx_location(2, :) - tx_location)];
                
                Gains_temp = zeros(rx_node_number, resolution);
                G_tx = zeros(rx_node_number, 1);
                G_interfer = zeros(rx_node_number, 1);
                for i = 1:rx_node_number
                    [weights, ~] = uniform(d, optimal_beam_direction(i), tx_antenna_number);
                    Gain = abs(dtft(weights, -psi)).^2;
                    Gains_temp(i, :) = Gain;

                    rx_Sector_idxs = round(AoD(i) * resolution / 180);
                    rx_interfer_Sector_idxs = round(AoD(3 - i) * resolution / 180);
                    [~, rx_Beam_idxs] = max(Gain(:, rx_Sector_idxs));
                    [~, rx_interfer_Beam_idxs] = max(Gain(:, rx_interfer_Sector_idxs));

                    G_tx(i) = Gain(rx_Beam_idxs, rx_Sector_idxs);
                    G_interfer(i) = Gain(rx_interfer_Beam_idxs, rx_interfer_Sector_idxs);
                    SNR_dB = P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i)) - N0_dBm;
                    
                    if (i == 1) 
                        printf(sprintf('Calculate Rx1 power and SNR\n'));
                        printf(sprintf('Reciever%d actual Angle: %.6f degree\n', i, AoD(i)));
                        printf(sprintf('Reciever%d power: %.6f dBm\n', i, P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i))));
                        printf(sprintf('Reciever%d SNR: %.6f dB\n\n', i, SNR_dB));
                    end
                    if (i == 2)
                        printf(sprintf('Calculate SINR of two concurrent beams\n'));
                        % write this part!
                        printf(sprintf('Rx1 interference power: %.6f dBm\n', P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1))));
                        printf(sprintf('Rx1 SINR: %.6f dB\n\n', P_tx_dBm + friis_equation(freq, G_tx(1), 1, rx_Dists(1)) - Wat2dBm(dBm2Wat(P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1))) + dBm2Wat(N0_dBm))));
                    end
                end
                Gains{a_idx, cb_idx} = Gains_temp;
            end
        end

        % Plot Cartesian
        figure;
        hold on;
        colors = {'b', 'r', 'g', 'm'};
        labels = {'4 Antennas, 5°', '4 Antennas, 15°', '8 Antennas, 5°', '8 Antennas, 15°'};
        for i = 1:4
            a_idx = ceil(i / 2);
            cb_idx = mod(i - 1, 2) + 1;
            plot(phi_d, Gains{a_idx, cb_idx}(1, :), 'Color', colors{i}, 'DisplayName', labels{i});
        end
        xlabel('Angle (degree)');
        ylabel('Gain (dB)');
        title('Tx Gain Comparison (User 1)');
        grid on;
        legend;
        hold off;

        figure;
        hold on;
        colors = {'b', 'r', 'g', 'm'};
        labels = {'4 Antennas, 5°', '4 Antennas, 15°', '8 Antennas, 5°', '8 Antennas, 15°'};
        for i = 1:4
            a_idx = ceil(i / 2);
            cb_idx = mod(i - 1, 2) + 1;
            plot(phi_d, Gains{a_idx, cb_idx}(2, :), 'Color', colors{i}, 'DisplayName', labels{i});
        end
        xlabel('Angle (degree)');
        ylabel('Gain (dB)');
        title('Tx Gain Comparison (User 2)');
        grid on;
        legend;
        hold off;

        % Plot Polar
        figure;
        ax = polaraxes; % 建立一個專用的 polaraxes
        hold on;
        for i = 1:4
            a_idx = ceil(i / 2);
            cb_idx = mod(i - 1, 2) + 1;
            p = polarplot(phi, Gains{a_idx, cb_idx}(1, :), 'Color', colors{i});
            set(p, 'DisplayName', labels{i});
        end
        title('Different antenna number Tx Gain in different directions (User 1)');
        legend('Location', 'best');
        hold off;

        figure;
        ax = polaraxes; % 建立一個專用的 polaraxes
        hold on;
        for i = 1:4
            a_idx = ceil(i / 2);
            cb_idx = mod(i - 1, 2) + 1;
            p = polarplot(phi, Gains{a_idx, cb_idx}(2, :), 'Color', colors{i});
            set(p, 'DisplayName', labels{i});
        end
        title('Different antenna number Tx Gain in different directions (User 2)');
        legend('Location', 'best');
        hold off;
    end
    function avg_20_random_run()
        % Environment Configurations
        freq = 24e9;
        rx_node_number = 2;
        d = 0.5;
        P_tx_dBm = 20;
        N0_dBm = -88;
        tx_location = [0, 0];

        resolution = 360;
        beam_codebooks = {[0:10:180]};
        antenna_numbers = [16];

        Rx1_powers = zeros(20, 1);
        Rx1_SNRs = zeros(20, 1);
        Rx1_interfer_powers = zeros(20, 1);
        Rx1_SINRs = zeros(20, 1);

        for run = 1:20
            % generate random rx_location
            rx_location = zeros(rx_node_number, 2);
            for i = 1:rx_node_number
                r = 5 + 20 * rand();    % Random distance between 5 and 25 meters (m)
                angle = 180 * rand();
                x = r * cosd(angle);    % Beam direction with a small random offset
                y = r * sind(angle);    % Beam direction with a small random offset
                rx_location(i, :) = [x, y];
            end

            % Compute optimal beam direction
            AoD = atan2d(rx_location(:,2) - tx_location(2), rx_location(:,1) - tx_location(1));
            AoD = mod(AoD, 180);

            % Initialize gain storage
            phi = (1 : resolution) * pi / resolution;
            phi_d = phi * 180 / pi;
            psi = 2 * pi * d * cos(phi);

            for a_idx = 1:length(antenna_numbers)
                tx_antenna_number = antenna_numbers(a_idx);
                for cb_idx = 1:length(beam_codebooks)
                    codebook = beam_codebooks{cb_idx};
                    [~, idx1] = min(abs(codebook - AoD(1)));
                    [~, idx2] = min(abs(codebook - AoD(2)));
                    optimal_beam_direction = [codebook(idx1), codebook(idx2)];

                    rx_Dists = [norm(rx_location(1, :) - tx_location), norm(rx_location(2, :) - tx_location)];
                    
                    G_tx = zeros(rx_node_number, 1);
                    G_interfer = zeros(rx_node_number, 1);
                    for i = 1:rx_node_number
                        [weights, ~] = uniform(d, optimal_beam_direction(i), tx_antenna_number);
                        Gain = abs(dtft(weights, -psi)).^2;

                        rx_Sector_idxs = round(AoD(i) * resolution / 180);
                        rx_interfer_Sector_idxs = round(AoD(3 - i) * resolution / 180);
                        [~, rx_Beam_idxs] = max(Gain(:, rx_Sector_idxs));
                        [~, rx_interfer_Beam_idxs] = max(Gain(:, rx_interfer_Sector_idxs));

                        G_tx(i) = Gain(rx_Beam_idxs, rx_Sector_idxs);
                        G_interfer(i) = Gain(rx_interfer_Beam_idxs, rx_interfer_Sector_idxs);
                        SNR_dB = P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i)) - N0_dBm;
                        
                        if (i == 1) 
                            % printf(sprintf('Calculate Rx1 power and SNR\n'));
                            % printf(sprintf('Reciever%d actual Angle: %.6f degree\n', i, AoD(i)));
                            % printf(sprintf('Reciever%d power: %.6f dBm\n', i, P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i))));
                            % printf(sprintf('Reciever%d SNR: %.6f dB\n\n', i, SNR_dB));
                            Rx1_powers(run) = P_tx_dBm + friis_equation(freq, G_tx(i), 1, rx_Dists(i));
                            Rx1_SNRs(run) = SNR_dB;
                        end
                        if (i == 2)
                            % printf(sprintf('Calculate SINR of two concurrent beams\n'));
                            % % write this part!
                            % printf(sprintf('Rx1 interference power: %.6f dBm\n', P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1))));
                            % printf(sprintf('Rx1 SINR: %.6f dB\n\n', P_tx_dBm + friis_equation(freq, G_tx(1), 1, rx_Dists(1)) - Wat2dBm(dBm2Wat(P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1))) + dBm2Wat(N0_dBm))));
                            Rx1_interfer_powers(run) = P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1));
                            Rx1_SINRs(run) = P_tx_dBm + friis_equation(freq, G_tx(1), 1, rx_Dists(1)) - Wat2dBm(dBm2Wat(P_tx_dBm + friis_equation(freq, G_interfer(2), 1, rx_Dists(1))) + dBm2Wat(N0_dBm));
                        end
                    end
                end
            end
        end
        printf(sprintf('avg Rx1 power: %.6f dBm\n', mean(Rx1_powers)));
        printf(sprintf('avg Rx1 SNR: %.6f dB\n', mean(Rx1_SNRs)));
        printf(sprintf('avg Rx1 interference power: %.6f dBm\n', mean(Rx1_interfer_powers)));
        printf(sprintf('avg Rx1 SINR: %.6f dB\n', mean(Rx1_SINRs)));
    end
    % ============== END of functions for supporting report ================