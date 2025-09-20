df -h .
rm -rf build
mkdir build

cmake -S. -Bbuild
cmake --build build -v
./build/bin/BasicExample

g++ -o BasicExample/src/lab3_greedy BasicExample/src/lab3_greedy.cc 
./BasicExample/src/lab3_greedy