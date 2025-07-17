#pragma once
#include <vector>
#include <array>

struct SurfaceMesh {
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<int, 4>> quads;
};
