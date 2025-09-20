close all; clear; clc;

n = randi([2, 10]);
A = rand(n, n);
y = rand(n, 1);

x = solveX(A, y);
y_ver = A*x;
disp('Random matrix A:'); disp(A);
disp('Random vector y:'); disp(y);
disp('Computed x:'); disp(x);
disp('Verification result y_ver:'); disp(y_ver);

if norm(y_ver - y) < 1e-6
    disp('Solution x is correct.');
else
    disp('Solution x is wrong.');
end
