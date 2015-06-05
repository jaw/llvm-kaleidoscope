clang++-3.6 -g toy.cpp `llvm-config-3.6 --cxxflags --ldflags --system-libs --libs core mcjit native` -O3 -o toy

