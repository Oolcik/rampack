//
// Created by Piotr Kubala on 12/12/2020.
//

#include <sstream>

#include "ShapeFactory.h"
#include "utils/Assertions.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"


namespace {
    template <typename ConcreteTraits, typename... Args>
    auto parse_polysphere_traits(const std::string &shapeName, const std::string &interactionName,
                                 std::istream &interactionAttrStream, Args&&... args)
    {
        if (interactionName.empty() || interactionName == "hard") {
            return std::make_shared<ConcreteTraits>(std::forward<Args>(args)...);
        } else if (interactionName == "lj") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed Lennard Jones attributes. Usage: lj [epsilon] [sigma]");
            Validate(epsilon > 0);
            Validate(sigma > 0);
            return std::make_shared<ConcreteTraits>(
                std::forward<Args>(args)..., std::make_unique<LennardJonesInteraction>(epsilon, sigma)
            );
        } else if (interactionName == "repulsive_lj") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed repulsive Lennard Jones attributes. Usage: lj_repulsive "
                                               "[epsilon] [sigma]");
            Validate(epsilon > 0);
            Validate(sigma > 0);
            return std::make_shared<ConcreteTraits>(
                std::forward<Args>(args)..., std::make_unique<RepulsiveLennardJonesInteraction>(epsilon, sigma)
            );
        } else {
            throw ValidationException(shapeName + " supports interactions: hard, lj (Lennard Jones), repulsive_lj "
                                                  "(Lennard Jones cut at the minimum)");
        }
    }
}


std::shared_ptr<ShapeTraits> ShapeFactory::shapeTraitsFor(const std::string &shapeName,
                                                          const std::string &shapeAttributes,
                                                          const std::string &interaction)
{
    std::istringstream shapeAttrStream(shapeAttributes);
    std::istringstream interactionAttrStream(interaction);
    std::string interactionName;
    interactionAttrStream >> interactionName;
    if (shapeName == "Sphere") {
        double r;
        shapeAttrStream >> r;
        ValidateMsg(shapeAttrStream, "Malformed Sphere attributes; expected: [radius]");
        Validate(r > 0);
        return parse_polysphere_traits<SphereTraits>(shapeName, interactionName, interactionAttrStream, r);
    } else if (shapeName == "PolysphereBanana") {
        double arcRadius, arcAngle, sphereRadius;
        std::size_t sphereNum;
        shapeAttrStream >> arcRadius >> arcAngle >> sphereNum >> sphereRadius;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereBanana attributes; expected: "
                                     "[arc radius] [arc angle] [number of spheres] [sphere radius]");
        Validate(arcRadius > 0);
        Validate(arcAngle > 0);
        Validate(sphereNum > 0);
        Validate(sphereRadius > 0);

        return parse_polysphere_traits<PolysphereBananaTraits>(shapeName, interactionName, interactionAttrStream,
                                                               arcRadius, arcAngle, sphereNum, sphereRadius);
    } else if (shapeName == "PolyspherocylinderBanana") {
        double arcRadius, arcAngle, radius;
        std::size_t segmentNum, subdivisions;
        shapeAttrStream >> arcRadius >> arcAngle >> segmentNum >> radius;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereBanana attributes; expected: "
                                     "[arc radius] [arc angle] [number of segments] [radius] (subdivisions = 1)");
        Validate(arcRadius > 0);
        Validate(arcAngle > 0);
        Validate(segmentNum > 0);
        Validate(radius > 0);

        shapeAttrStream >> subdivisions;
        if (!shapeAttrStream)
            subdivisions = 1;
        else
            Validate(subdivisions > 0);

        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "SpherocylinderBanana supports only hard interactions");

        return std::make_shared<PolyspherocylinderBananaTraits>(arcRadius, arcAngle, segmentNum, radius, subdivisions);
    } else if (shapeName == "KMer") {
        double sphereRadius, distance;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> sphereRadius >> distance;
        ValidateMsg(shapeAttrStream, "Malformed KMer attributes; expected: "
                                     "[number of spheres] [sphere radius] [distance between spheres]");
        Validate(sphereNum >= 2);
        Validate(sphereRadius > 0);
        Validate(distance > 0);

        return parse_polysphere_traits<KMerTraits>(shapeName, interactionName, interactionAttrStream, sphereNum,
                                                   sphereRadius, distance);
    } else if (shapeName == "PolysphereLollipop") {
        double smallSphereRadius, largeSphereRadius, smallSpherePenetration, largeSpherePenetration;
        std::size_t sphereNum;
        shapeAttrStream >> sphereNum >> smallSphereRadius >> largeSphereRadius >> smallSpherePenetration;
        shapeAttrStream >> largeSpherePenetration;
        ValidateMsg(shapeAttrStream, "Malformed PolysphereLollipop attributes; expected: "
                                     "[number of spheres] [small sphere radius] [large sphere radius] "
                                     "[small spheres penetration] [large sphere penetration]");
        Validate(sphereNum >= 2);
        Validate(smallSphereRadius > 0);
        Validate(largeSphereRadius > 0);
        Validate(smallSpherePenetration < 2*smallSphereRadius);
        Validate(largeSpherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

        return parse_polysphere_traits<PolysphereLollipopTraits>(shapeName, interactionName, interactionAttrStream,
                                                                 sphereNum, smallSphereRadius, largeSphereRadius,
                                                                 smallSpherePenetration, largeSpherePenetration);
    } else if (shapeName == "Spherocylinder") {
        double r, length;
        shapeAttrStream >> length >> r;
        ValidateMsg(shapeAttrStream, "Malformed Spherocylinder attributes; expected: [length] [radius]");
        Validate(r > 0);
        Validate(length >= 0);
        ValidateMsg(interactionName == "hard" || interactionName.empty(),
                    "Spherocylinder supports only hard interactions");
        return std::make_shared<SpherocylinderTraits>(length, r);
    } else {
        throw ValidationException("Unknown particle name: " + shapeName);
    }
}
