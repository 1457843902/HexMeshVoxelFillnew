#pragma once
#include <array>

// 六面体单元由8个点索引构成
struct Hexahedron {
    std::array<int, 8> indices;
};
