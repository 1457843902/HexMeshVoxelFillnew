
#include <algorithm>          // for std::min/std::max
#include <cmath>              // for std::abs, std::sqrt
#include "IO.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

SurfaceMesh load_surface_obj(const std::string& filename) {
    SurfaceMesh mesh;
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return mesh;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            mesh.vertices.push_back({ x, y, z });
        } else if (type == "f") {
            int a, b, c, d;
            iss >> a >> b >> c >> d;
            mesh.quads.push_back({ a - 1, b - 1, c - 1, d - 1 });
        }
    }

    return mesh;
}

void write_vtk(const std::string& filename,
               const std::vector<std::array<float, 3>>& vertices,
               const std::vector<Hexahedron>& hexes) {
    std::ofstream out(filename);
    out << "# vtk DataFile Version 3.0\n";
    out << "Hex Mesh\n";
    out << "ASCII\n";
    out << "DATASET UNSTRUCTURED_GRID\n";

    out << "POINTS " << vertices.size() << " float\n";
    for (const auto& v : vertices)
        out << v[0] << " " << v[1] << " " << v[2] << "\n";

    out << "CELLS " << hexes.size() << " " << (9 * hexes.size()) << "\n";
    for (const auto& h : hexes) {
        out << "8 ";
        for (int i = 0; i < 8; ++i)
            out << h.indices[i] << " ";
        out << "\n";
    }

    out << "CELL_TYPES " << hexes.size() << "\n";
    for (size_t i = 0; i < hexes.size(); ++i)
        out << "12\n";
}
