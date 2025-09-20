%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DO NOT MODIFY THIS SECTION. Any changes will result in a penalty.
close all; clear; clc;
addpath ./ewa_function;
rng(77);

% Environment Configurations
freq = 24e9;
tx_antenna_number = 16;  % Number of Tx antennas per ground station
% beam angles (degrees) to sweep
tx_beam_direction = 0:5:90;
d = 0.5;                 % Antenna spacing (wavelength multiple)
P_tx_dBm = 20;           % Transmission power of Tx (dBm)
N0_dBm = -88;            % Noise power (dBm)
resolution = 360;        % Number of angular samples for array pattern
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fprintf('Task 1: Generate network.graph\n');

%--- 1) Read network.pos ------------------------------------------------
fid = fopen('network.pos','r');
params = fscanf(fid, '%f', 5)';            % P_tx, N0, freq, bandwidth, rx_thresh
P_tx_dBm       = params(1);
N0_dBm         = params(2);
freq           = params(3);
bandwidth_Hz   = params(4);	rx_thresh_dBm  = params(5);
counts = fscanf(fid, '%d', 2)';           % V (#ground), S (#sat)
V = counts(1);
S = counts(2);
% Read V ground stations: [id, x, y, z]
gs = fscanf(fid, '%d %f %f %f', [4, V])';
% Read S satellites:      [id, x, y, z]
sat = fscanf(fid, '%d %f %f %f', [4, S])';
fclose(fid);

% Precompute angular grid for array pattern
phi = (1:resolution) * pi / resolution;        % radians
psi = 2 * pi * d * sin(phi);                   % spatial frequency vector
beam_resolutions = (0.5:(180/resolution):90);  % beam angle bins (deg)

links = [];

for i = 1:V
    v_id = gs(i,1);
    v_pos = gs(i,2:4);
    for j = 1:S
        s_id = sat(j,1);
        s_pos = sat(j,2:4);

        %--- geometry ----------------------------------------------------
        horiz = norm(v_pos(1:2) - s_pos(1:2));
        vert  = abs(s_pos(3) - v_pos(3));
        total_dist = sqrt(horiz^2 + vert^2);
        % elevation angle (deg)
        elev = atan2d(horiz, vert);
        if elev >= 90, elev = 180 - elev; end
        % select closest tx beam direction
        [~, tx_idx] = min(abs(tx_beam_direction - elev));
        % find AoD resolution index
        [~, res_idx] = min(abs(beam_resolutions - elev));

        %--- compute array gain -------------------------------------------
        [weights, ~] = uniform(d, tx_beam_direction(tx_idx), tx_antenna_number);
        GainMat = abs(dtft(weights, -psi)).^2;  % size: antennas x resolution
        % pick strongest beam index at that elevation slice
        [~, beam_idx] = max(GainMat(:, res_idx));
        G_tx_lin = GainMat(beam_idx, res_idx);

        %--- compute Friis path gain --------------------------------------
        Friis_dB = friis_equation(freq, G_tx_lin, 1, total_dist);
        Prx_dBm  = P_tx_dBm + Friis_dB;
        % link too weak
        if Prx_dBm < rx_thresh_dBm, continue; end

        %--- compute data rate --------------------------------------------
        Pnoise_W = 10^((N0_dBm - 30)/10);
        Prx_W    = 10^((Prx_dBm  - 30)/10);
        SNR_lin  = Prx_W / Pnoise_W;
        C_bps    = bandwidth_Hz * log2(1 + SNR_lin);
        C_kbps   = C_bps / 1e3;
        % store
        links(end+1,:) = [v_id, s_id, C_kbps];  %#ok<AGROW>
    end
end

%--- write network.graph -----------------------------------------------
outFID = fopen('network.graph','w');
fprintf(outFID, '%d %d %d\n', V, S, size(links,1));
for k = 1:size(links,1)
    fprintf(outFID, '%d %d %.6f\n', links(k,1), links(k,2), links(k,3));
end
fclose(outFID);

fprintf('Output network.graph with %d links written.\n', size(links,1));

%=== supporting function =================================================
function gain_dB = friis_equation(freq_Hz, Gtx_lin, Grx_lin, d_m)
    c      = 299792458;
    lambda = c / freq_Hz;
    fspl   = (lambda / (4*pi*d_m))^2;
    gain_dB = 10*log10(Gtx_lin * Grx_lin * fspl);
end
