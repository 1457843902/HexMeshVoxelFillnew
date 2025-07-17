cmake_minimum_required(VERSION 3.10)
project(HexMeshVoxelFillnew)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(demo
    src/demo.cpp
    src/Voxelizer.cpp
    src/IO.cpp
)
