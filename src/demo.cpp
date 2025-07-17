
#include "SurfaceMesh.hpp"
#include "Voxelizer.hpp"
#include "IO.hpp"
#include <iostream>

int main() {
    SurfaceMesh surface = load_surface_obj("D:/code/HexMeshVoxelFillnew/data/cube.obj");
    std::cout << "Loaded surface: " << surface.vertices.size()
        << " vertices, " << surface.quads.size() << " quads.\n";

    /* === 调试：打印环大小 === */
    auto top_loop = extract_top_loop(surface);
    auto bottom_loop = extract_bottom_loop(surface);
    std::cout << "top_loop size = " << top_loop.size()
        << ", bottom_loop size = " << bottom_loop.size() << '\n';

    auto [vertices, hexes] = sweep_hex_mesh(surface, 10); // 10 层
    std::cout << "Generated " << hexes.size() << " hexahedra, "<< vertices.size() << " vertices.\n";

    write_vtk("D:/code/HexMeshVoxelFillnew/output.vtk", vertices, hexes);
    std::cout << "Written to output.vtk\n";
    return 0;
}