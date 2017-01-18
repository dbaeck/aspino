cxxtestgen --error-printer -o runner.cpp aspino-test/AlgorithmTests.h
g++ -o runner -I$CXXTEST -i ./runner.cpp -I/usr/local/include/ -Isrc/glucose-syrup/ -Isrc/ -lm -lz
./runner