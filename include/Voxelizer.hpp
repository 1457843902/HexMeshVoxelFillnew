#pragma once
#include "Hexahedron.hpp"
#include "SurfaceMesh.hpp"
#include <vector>
#include <array>

struct VoxelResult {
    std::vector<std::array<float, 3>> vertices;
    std::vector<Hexahedron> hexes;
};

VoxelResult voxel_fill(const SurfaceMesh& surface, float resolution = 0.1f);

VoxelResult sweep_hex_mesh(const SurfaceMesh& surface, int layers);
// 从四边形表面提取顶部/底部轮廓（按逆时针排序）
std::vector<std::array<float, 3>> extract_top_loop(const SurfaceMesh& mesh);
std::vector<std::array<float, 3>> extract_bottom_loop(const SurfaceMesh& mesh);
