//
// Created by ciesla on 12/04/23.
//

#include "DistortedTetrahedronTraits.h"
#include "utils/Exceptions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double DistortedTetrahedronTraits::getVolume(double rxUp, double ryUp, double rxDown, double ryDown, double l) {
    double v = (2.0*l/3.0)*(2*rxDown*ryDown + rxUp*ryDown + rxDown*ryUp + 2*rxUp*ryUp);
    return v;
}

DistortedTetrahedronTraits::DistortedTetrahedronTraits(double rxUp, double ryUp, double rxDown, double ryDown, double l, std::size_t subdivisions)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            DistortedTetrahedronTraits::getVolume(rxUp, ryUp, rxDown, ryDown, l),
                            {{"beg", {0, 0, -l/2}}, {"end", {0, 0, l/2}}}),
                            rxUp{rxUp}, ryUp{ryUp}, rxDown{rxDown}, ryDown{ryDown}, l{l}
{
    Expects(rxUp >= 0);
    Expects(ryUp > 0);
    Expects(rxDown > 0);
    Expects(ryDown >= 0);
    Expects(l > 0);

    if (subdivisions == 0 || subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeModel.emplace_back(this->rxUp, this->ryUp, this->rxDown, this->ryDown, this->l);
        return;
    }

    double dl = this->l/static_cast<double>(subdivisions);
    double dy = (this->ryDown - this->ryUp)/static_cast<double>(subdivisions);
    double dx = (this->rxDown - this->rxUp)/static_cast<double>(subdivisions);
    this->shapeModel.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double rxUp = this->rxUp + static_cast<double>(i)*dx;
        double ryUp = this->ryUp + static_cast<double>(i)*dy;
        double rxDown = this->rxUp + static_cast<double>(i+1)*dx;
        double ryDown = this->ryUp + static_cast<double>(i+1)*dy;

        if (rxUp > std::max(this->rxUp, this->rxDown)) rxUp = std::max(this->rxUp, this->rxDown);
        if (rxUp < std::min(this->rxUp, this->rxDown)) rxUp = std::min(this->rxUp, this->rxDown);;

        if (rxDown > std::max(this->rxUp, this->rxDown)) rxDown = std::max(this->rxUp, this->rxDown);
        if (rxDown < std::min(this->rxUp, this->rxDown)) rxDown = std::min(this->rxUp, this->rxDown);;

        if (ryUp > std::max(this->ryUp, this->ryDown)) ryUp = std::max(this->ryUp, this->ryDown);
        if (ryUp < std::min(this->ryUp, this->ryDown)) ryUp = std::min(this->ryUp, this->ryDown);

        if (ryDown > std::max(this->ryUp, this->ryDown)) ryDown = std::max(this->ryUp, this->ryDown);
        if (ryDown < std::min(this->ryUp, this->ryDown)) ryDown = std::min(this->ryUp, this->ryDown);

        this->shapeModel.emplace_back(rxUp, ryUp, rxDown, ryDown, dl);
        this->interactionCentres.push_back({0, 0, l/2.0-(static_cast<double>(i)+0.5)*dl});
    }
}


std::shared_ptr<const ShapePrinter>
DistortedTetrahedronTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    // We override the function from XenoCollideTraits not to redundantly print subdivisions

    std::size_t meshSubdivisions = XenoCollideTraits::DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->createPrinter<XCWolframShapePrinter>(meshSubdivisions);
    else if (format == "obj")
        return this->createPrinter<XCObjShapePrinter>(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
}

DistortedTetrahedronTraits::CollideGeometry::CollideGeometry(double rxUp, double ryUp, double rxDown, double ryDown, double l)
        : rxUp{rxUp}, ryUp{ryUp}, rxDown{rxDown}, ryDown{ryDown}, l{l}
{
    Expects(rxUp >= 0);
    Expects(ryUp > 0);
    Expects(rxDown > 0);
    Expects(ryDown >= 0);
    Expects(l > 0);

    double crUp = std::sqrt(rxUp*rxUp + ryUp*ryUp + l*l/4.0);
    double crDown = std::sqrt(rxDown*rxDown + ryDown*ryDown + l*l/4.0);
    this->circumsphereRadius = std::max(crUp, crDown);

    double irUp = std::min(rxUp, ryUp);
    double irDown = std::min(rxDown, ryDown);
    this->insphereRadius = std::min(std::min(irUp, irDown), l/2.0);
}
