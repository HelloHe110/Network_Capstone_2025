function dph = bwidth(d, ph0, dps)
    % bwidth.m - 計算天線陣列的 3-dB 波束寬度
    %
    % Usage: dph = bwidth(d, ph0, dps)
    %
    % d   = 天線間距 (單位: 波長倍數)
    % ph0 = 波束方向 (單位: degree)
    % dps = 波束標準寬度 (radian)
    %
    % dph = 3-dB 波束寬度 (degree)
    
    if nargin == 0
        help bwidth; return;
    end
    
    % 轉換 ph0 到 rad
    ph0_rad = deg2rad(ph0);
    
    % 計算 3-dB 波束寬度 (degree)
    dph = rad2deg(dps / cos(ph0_rad));
end
    