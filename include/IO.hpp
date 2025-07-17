#pragma once
#include "SurfaceMesh.hpp"
#include "Hexahedron.hpp"
#include <string>
#include <vector>

SurfaceMesh load_surface_obj(const std::string& filename);
void write_vtk(const std::string& filename,
               const std::vector<std::array<float, 3>>& vertices,
               const std::vector<Hexahedron>& hexes);
