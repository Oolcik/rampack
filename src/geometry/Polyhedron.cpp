//
// Created by pkua on 17.11.22.
//

#include "Polyhedron.h"


std::string Polyhedron::toWolfram() const {
    std::ostringstream out;
    out << std::fixed;

    out << "GraphicsComplex[{" << std::endl;
    for (std::size_t i{}; i < this->vertices.size(); i++) {
        out << "    " << this->vertices[i];
        if (i < this->vertices.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "}," << std::endl << "Polygon[{" << std::endl;
    for (std::size_t i{}; i < this->triangles.size(); i++) {
        const auto &tri = this->triangles[i];
        out << "    {" << (tri[0] + 1) << ", " << (tri[1] + 1) << ", " << (tri[2] + 1) << "}";
        if (i < this->triangles.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "}]]" << std::endl;

    return out.str();
}

double Polyhedron::getVolume() const {
    double vol{};

    for (const auto &triangle : this->triangles) {
        Vector<3> v1 = this->vertices[triangle[2]] - this->vertices[triangle[0]];
        Vector<3> v2 = this->vertices[triangle[2]] - this->vertices[triangle[1]];
        Vector<3> v3 = this->vertices[triangle[2]] - this->center;
        vol += 1./6 * std::abs((v1 ^ v2) * v3);
    }

    return vol;
}

void Polyhedron::storeWavefrontObj(std::ostream &out) const {
    for (const auto &v : this->vertices)
        out << "v " << v[0] << " " << v[1] << " " << v[2] << std::endl;
    out << std::endl;

    for (const auto &tri : this->triangles)
        out << "f " << (tri[0] + 1) << " " << (tri[1] + 1) << " " << (tri[2] + 1) << std::endl;
}
