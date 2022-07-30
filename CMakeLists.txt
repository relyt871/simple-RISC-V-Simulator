cmake_minimum_required(VERSION 3.16)
project(simple_riscv_simulator)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Ofast)
add_executable(simple_riscv_simulator src/main.cpp)