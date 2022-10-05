//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_XENOCOLLIDETRAITS_H
#define RAMPACK_XENOCOLLIDETRAITS_H

#include <map>
#include <utility>
#include <sstream>

#include "core/ShapeTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/xenocollide/XenoCollide.h"
#include "geometry/xenocollide/XCPrinter.h"


template<typename ConcreteCollideTraits>
class XenoCollideTraits : public ShapeTraits, public Interaction, public ShapeGeometry, public ShapePrinter {
private:
    Vector<3> primaryAxis;
    Vector<3> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> customNamedPoints{};

public:
//    XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, const std::string &attr,
//                      std::map<std::string, Vector<3>> customNamedPoints = {})
//            : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{cm}, volume{v},
//              customNamedPoints{std::move(customNamedPoints)}
//    {
//        std::stringstream ss(attr);
//        std::string commands;
//        std::getline(ss, commands, '\0');
//        std::string script = "script " + commands;
//
//        BodyBuilder bb;
//        bb.ProcessCommand(script);
//        this->shapeModel = bb.getCollideGeometry();
//        this->rangeRadius = 2*bb.getMaxRadius();
//    }


    XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> o, double v,
                      std::map<std::string, Vector<3>> customNamedPoints = {})
            : primaryAxis{pa}, secondaryAxis{sa}, geometricOrigin{o}, volume{v},
              customNamedPoints{std::move(customNamedPoints)}
    {

    }

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->primaryAxis;
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->secondaryAxis;
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        return shape.getOrientation() * this->geometricOrigin;
    }

    [[nodiscard]] double getVolume() const final { return this->volume; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override {
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry1 = thisConcreteTraits.getCollideGeometry(idx1);
        const auto &collideGeometry2 = thisConcreteTraits.getCollideGeometry(idx2);
        using XCGeometry = decltype(collideGeometry1);
        double rangeRadius = collideGeometry1.getCircumsphereRadius() + collideGeometry2.getCircumsphereRadius();

        Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
        if ((pos2bc - pos1).norm2() > rangeRadius*rangeRadius)
            return false;
        bool result = XenoCollide<XCGeometry>::Intersect(collideGeometry1, orientation1, pos1, collideGeometry2, orientation2, pos2bc, 1.0e-12);
        return result;
    }

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, [[maybe_unused]] std::size_t idx,
                                       const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override {

        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry = thisConcreteTraits.getCollideGeometry(idx);

        Vector<3> normalVector = (orientation.transpose())*(wallVector);
        Vector<3> sp = collideGeometry.getSupportPoint(-normalVector);
        Vector<3> origin = (orientation.transpose())*(wallOrigin - pos);
        double distanceSupport = -sp*normalVector;  // minus sign because we count distance along -normalVector
        double distanceWall = -origin*normalVector;
        if (distanceWall > distanceSupport)
            return false;
        return true;
    }

    [[nodiscard]] double getRangeRadius() const override {
        std::size_t numCenters = std::max(this->getInteractionCentres().size(), 1ul);
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        double maxRadius{};
        for (std::size_t i{}; i < numCenters; i++) {
            double newRadius = thisConcreteTraits.getCollideGeometry(i).getCircumsphereRadius();
            if (newRadius > maxRadius)
                maxRadius = newRadius;
        }
        return 2*maxRadius;
    }

    [[nodiscard]] Vector<3> getNamedPoint(const std::string &pointName, const Shape &shape) const override {
        auto namedPoint = this->customNamedPoints.find(pointName);
        if (namedPoint == this->customNamedPoints.end())
            return ShapeGeometry::getNamedPoint(pointName, shape);

        return shape.getPosition() + shape.getOrientation() * namedPoint->second;
    }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override {
        std::ostringstream out;
        out << "{EdgeForm[None]," << std::endl;

        auto centers = this->getInteraction().getInteractionCentres();
        if (centers.empty())
            centers.push_back({0, 0, 0});

        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        Matrix<3, 3> orientation = shape.getOrientation();
        for (std::size_t i{}; i < centers.size(); i++) {
            const auto &geometry = thisConcreteTraits.getCollideGeometry(i);
            const auto &center = centers[i];
            auto polyhedron = XCPrinter::buildPolyhedron(geometry, 2);
            std::string wolframPolyhedron = polyhedron.toWolfram();

            Vector<3> pos = shape.getPosition() + orientation * center;

            out << "GeometricTransformation[" << wolframPolyhedron << "," << std::endl;
            out << "AffineTransform[" << std::endl;
            out << "    {{{" << orientation(0, 0) << ", " << orientation(0, 1) << ", " << orientation(0, 2) << "}," << std::endl;
            out << "      {" << orientation(1, 0) << ", " << orientation(1, 1) << ", " << orientation(1, 2) << "}," << std::endl;
            out << "      {" << orientation(2, 0) << ", " << orientation(2, 1) << ", " << orientation(2, 2) << "}}," << std::endl;
            out << "      " << pos << "}]" << std::endl;
            out << "]";

            if (i < centers.size() - 1)
                out << ",";
            out << std::endl;
        }
        out << "}";

        return out.str();
    }
};


#endif //RAMPACK_XENOCOLLIDETRAITS_H
