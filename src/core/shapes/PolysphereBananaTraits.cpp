//
// Created by Piotr Kubala on 22/12/2020.
//

#include "PolysphereBananaTraits.h"
#include "utils/Assertions.h"


PolysphereBananaTraits::PolysphereGeometry
PolysphereBananaTraits::generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(arcAngle < 2*M_PI);
    Expects(sphereNum >= 2);
    Expects(sphereRadius > 0);

    double angleStep = arcAngle/static_cast<double>(sphereNum - 1);
    std::vector<Vector<3>> spherePos;
    spherePos.reserve(sphereNum);
    double angle = -arcAngle/2;
    for (std::size_t i{}; i < sphereNum; i++) {
        Vector<3> pos = Matrix<3, 3>::rotation(0, angle, 0) * Vector<3>{-arcRadius, 0, 0};
        spherePos.push_back(pos);
        angle += angleStep;
    }

    if (arcAngle < M_PI) {
        Vector<3> translation = {-spherePos.front()[0], 0, 0};
        for (auto &pos : spherePos)
            pos += translation;
    }

    std::vector<PolysphereTraits::SphereData> sphereData;
    sphereData.reserve(sphereNum);
    for (const auto &pos : spherePos)
        sphereData.emplace_back(pos, sphereRadius);

    PolysphereGeometry geometry(std::move(sphereData), {0, 0, 1}, {-1, 0, 0});
    geometry.addCustomNamedPoints({{"beg", spherePos.front()}, {"end", spherePos.back()}});
    return geometry;
}
