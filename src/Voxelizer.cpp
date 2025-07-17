#include <algorithm>
#include <cmath>
#include "Voxelizer.hpp"
#include <unordered_set>
#include <array>
#include <vector>
#include "SurfaceMesh.hpp"
#include <map>
#include <set>

// 把四边形环当轨道，沿直线一层层‘复印’，每层 8 个点连成砖，砖连成片，片就是六面体网格

// 把 std::array<int,3> 变成哈希键，用于后续 空间去重
struct Vec3iHash {
    size_t operator()(const std::array<int, 3>& v) const {
        return std::hash<int>()(v[0]) ^

            //经典质数，降低哈希碰撞
            std::hash<int>()(v[1] * 73856093) ^
            std::hash<int>()(v[2] * 19349663);
    }
};

bool is_inside_bbox(const std::array<float, 3>& p,
    const std::array<float, 3>& min,
    const std::array<float, 3>& max) {
    return (p[0] >= min[0] && p[0] <= max[0] &&
        p[1] >= min[1] && p[1] <= max[1] &&
        p[2] >= min[2] && p[2] <= max[2]);
}

VoxelResult voxel_fill(const SurfaceMesh& surface, float resolution) {
    VoxelResult result;
    std::vector<Hexahedron> hexes;
    std::unordered_set<std::array<int, 3>, Vec3iHash> filled;

    if (surface.vertices.empty()) {
        return result;
    }

    std::array<float, 3> bbox_min = surface.vertices[0];
    std::array<float, 3> bbox_max = surface.vertices[0];

    for (const auto& v : surface.vertices) {
        for (int i = 0; i < 3; ++i) {
            bbox_min[i] = std::min(bbox_min[i], v[i]);
            bbox_max[i] = std::max(bbox_max[i], v[i]);
        }
    }

    int nx = static_cast<int>((bbox_max[0] - bbox_min[0]) / resolution);
    int ny = static_cast<int>((bbox_max[1] - bbox_min[1]) / resolution);
    int nz = static_cast<int>((bbox_max[2] - bbox_min[2]) / resolution);

    std::vector<std::array<float, 3>> grid_points;

    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                std::array<float, 3> base = {
                    bbox_min[0] + i * resolution,
                    bbox_min[1] + j * resolution,
                    bbox_min[2] + k * resolution
                };

                int idx = static_cast<int>(grid_points.size());

                for (int dz = 0; dz <= 1; ++dz) {
                    for (int dy = 0; dy <= 1; ++dy) {
                        for (int dx = 0; dx <= 1; ++dx) {
                            grid_points.push_back({
                                base[0] + dx * resolution,
                                base[1] + dy * resolution,
                                base[2] + dz * resolution
                                });
                        }
                    }
                }

                Hexahedron h;
                for (int q = 0; q < 8; ++q) h.indices[q] = idx + q;
                hexes.push_back(h);
            }
        }
    }
    result.vertices = std::move(grid_points);
    result.hexes = std::move(hexes);

    return result;
}

// 判断三点是否共线（简单距离法）
static bool almost_collinear(const std::array<float, 3>& a,
    const std::array<float, 3>& b,
    const std::array<float, 3>& c,
    float eps = 1e-4f)
{
    std::array<float, 3> ab{ b[0] - a[0], b[1] - a[1], b[2] - a[2] };
    std::array<float, 3> ac{ c[0] - a[0], c[1] - a[1], c[2] - a[2] };
    float cx = ab[1] * ac[2] - ab[2] * ac[1];
    float cy = ab[2] * ac[0] - ab[0] * ac[2];
    float cz = ab[0] * ac[1] - ab[1] * ac[0];
    return std::sqrt(cx * cx + cy * cy + cz * cz) < eps;
}

// 提取顶部或底部环顶点（按角度排序）
static std::vector<int> extract_loop_indices(const SurfaceMesh& mesh, bool want_top)
{
    if (mesh.vertices.empty()) return {};

    // 1. 找极值 Z
    float z_ext = want_top ? mesh.vertices[0][2] : mesh.vertices[0][2];
    for (const auto& v : mesh.vertices)
        z_ext = want_top ? std::max(z_ext, v[2]) : std::min(z_ext, v[2]);

    // 2. 收集所有落在该 Z 面的顶点
    const float eps = 1e-3f;
    std::vector<int> loop;
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
        if (std::abs(mesh.vertices[i][2] - z_ext) < eps)
            loop.push_back(static_cast<int>(i));

    // 3. 按俯视角度排序
    auto center = [&](const std::vector<int>& ids) {
        std::array<float, 3> c{ 0,0,0 };
        for (int id : ids)
            for (int k = 0; k < 2; ++k) c[k] += mesh.vertices[id][k];
        for (int k = 0; k < 2; ++k) c[k] /= ids.size();
        return c;
        };
    auto c = center(loop);
    std::sort(loop.begin(), loop.end(), [&](int a, int b) {
        const auto& va = mesh.vertices[a];
        const auto& vb = mesh.vertices[b];
        return std::atan2(va[1] - c[1], va[0] - c[0]) <
            std::atan2(vb[1] - c[1], vb[0] - c[0]);
        });
    return loop;
}

// 包装成顶层可用的接口 方便外部直接使用

std::vector<std::array<float, 3>> extract_top_loop(const SurfaceMesh& mesh) {
    auto ids = extract_loop_indices(mesh, true);
    std::vector<std::array<float, 3>> loop;
    for (int id : ids) loop.push_back(mesh.vertices[id]);
    return loop;
}
std::vector<std::array<float, 3>> extract_bottom_loop(const SurfaceMesh& mesh) {
    auto ids = extract_loop_indices(mesh, false);
    std::vector<std::array<float, 3>> loop;
    for (int id : ids) loop.push_back(mesh.vertices[id]);
    return loop;
}

//（扫掠六面体网格）
VoxelResult sweep_hex_mesh(const SurfaceMesh& surface, int layers) {
    // Step 1: 提取顶部和底部轮廓
    auto top_loop = extract_top_loop(surface);
    auto bottom_loop = extract_bottom_loop(surface);

    // Step 2: 插值层
    VoxelResult result;
    int n = static_cast<int>(top_loop.size());

    // 生成顶点
    for (int l = 0; l <= layers; ++l) {
        float t = float(l) / layers;
        for (int i = 0; i < n; ++i) {
            const auto& [xt, yt, zt] = top_loop[i];
            const auto& [xb, yb, zb] = bottom_loop[i];
            float x = xt * (1 - t) + xb * t;
            float y = yt * (1 - t) + yb * t;
            float z = zt * (1 - t) + zb * t;
            result.vertices.push_back({ x, y, z });
        }
    }

    // 生成六面体单元
    for (int l = 0; l < layers; ++l) {
        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            int v0 = l * n + i;
            int v1 = l * n + next;
            int v2 = (l + 1) * n + next;
            int v3 = (l + 1) * n + i;
            int v4 = v0 + n;
            int v5 = v1 + n;
            int v6 = v2 + n;
            int v7 = v3 + n;

            Hexahedron h;
            h.indices = { v0, v1, v2, v3, v4, v5, v6, v7 };
            result.hexes.push_back(h);
        }
    }
    // 在返回前，整体放大 10 倍并平移
    for (auto& v : result.vertices) {
        v[0] *= 10.0f;
        v[1] *= 10.0f;
        v[2] *= 10.0f;
        v[0] += 100.0f;
        v[1] += 100.0f;
        v[2] += 100.0f;
    }
    return result;
}