//
// Created by pkua on 22.02.2022.
//

#include <iostream>

#include "TriclinicBoxScalerFactory.h"
#include "core/VolumeScaler.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/AnisotropicVolumeScaler.h"
#include "core/volume_scalers/LinearScalingFactorSampler.h"
#include "core/volume_scalers/LogScalingFactorSampler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/volume_scalers/TriclinicDeltaScaler.h"

#define SCALING_USAGE "Malformed scaling. Available types: delta V, (independent) linear, (independent) log, " \
                      "(independent) delta triclinic"

#define SCALING_DIRECTION_USAGE "Malformed scaling direction. Alternatives: \n" \
                                "1. isotropic \n"                               \
                                "2. anisotropic x|y|z \n"                       \
                                "3. anisotropic xyz \n"                         \
                                "4. y[xz] | (xy)z | [x]yz | etc. (where (...) - scale together, [...] - do not scale)"

namespace {
    AnisotropicVolumeScaler::ScalingDirection char_to_scaling_direction(char c) {
        switch (c) {
            case 'x':
                return AnisotropicVolumeScaler::X;
            case 'y':
                return AnisotropicVolumeScaler::Y;
            case 'z':
                return AnisotropicVolumeScaler::Z;
            default:
                throw AssertionException({c});
        }
    }

    std::unique_ptr<VolumeScaler> create_volume_scaler(std::string scalingType) {
        if (scalingType == "delta V")
            return std::make_unique<DeltaVolumeScaler>();

        std::istringstream scalingTypeStream(scalingType);
        std::string independentString = "independent ";
        bool independent = startsWith(scalingType, independentString);
        if (independent)
            scalingTypeStream >> independentString;

        std::string scalerName;
        scalingTypeStream >> scalerName;
        ValidateMsg(scalingTypeStream, SCALING_USAGE);

        std::unique_ptr<ScalingFactorSampler> factorSampler;
        if (scalerName == "linear")
            factorSampler = std::make_unique<LinearScalingFactorSampler>();
        else if (scalerName == "log")
            factorSampler = std::make_unique<LogScalingFactorSampler>();
        else
            throw ValidationException(SCALING_USAGE);

        std::string scalingDirectionStr;
        scalingTypeStream >> std::ws;
        std::getline(scalingTypeStream, scalingDirectionStr);
        ValidateMsg(scalingTypeStream, SCALING_DIRECTION_USAGE);

        const auto &X = AnisotropicVolumeScaler::X;
        const auto &Y = AnisotropicVolumeScaler::Y;
        const auto &Z = AnisotropicVolumeScaler::Z;

        if (scalingDirectionStr == "isotropic")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), X & Y & Z, independent);
        else if (scalingDirectionStr == "anisotropic x")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), X | (Y & Z), independent);
        else if (scalingDirectionStr == "anisotropic y")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), Y | (Z & X), independent);
        else if (scalingDirectionStr == "anisotropic z")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), Z | (X & Y), independent);
        else if (scalingDirectionStr == "anisotropic xyz")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), X | Y | Z, independent);

        AnisotropicVolumeScaler::ScalingDirection scalingDirection;
        std::array<bool, 3> directionUsed{};
        for (std::size_t i{}; i < scalingDirectionStr.length(); i++) {
            char c = scalingDirectionStr[i];
            switch (c) {
                case 'x':
                case 'y':
                case 'z': {
                    std::size_t directionIdx = c - 'x';
                    if (directionUsed[directionIdx])
                        throw ValidationException("Duplicated occurrence of " + std::string{c} + " direction");
                    directionUsed[directionIdx] = true;
                    scalingDirection |= char_to_scaling_direction(c);
                    break;
                }
                case '(': {
                    std::size_t closingIdx = scalingDirectionStr.find(')', i);
                    if (closingIdx == std::string::npos)
                        throw ValidationException("Unmatched '(' in scaling direction");
                    std::string groupedDirections = scalingDirectionStr.substr(i + 1, closingIdx - i - 1);
                    i = closingIdx;

                    AnisotropicVolumeScaler::ScalingDirection groupedScalingDirection;
                    for (char direction : groupedDirections) {
                        if (direction < 'x' && direction > 'z')
                            throw ValidationException(SCALING_USAGE);
                        std::size_t directionIdx = direction - 'x';
                        if (directionUsed[directionIdx])
                            throw ValidationException("Duplicated occurrence of " + std::string{direction} + " direction");
                        directionUsed[directionIdx] = true;
                        groupedScalingDirection &= char_to_scaling_direction(direction);
                    }
                    scalingDirection |= groupedScalingDirection;
                    break;
                }

                case '[': {
                    std::size_t closingIdx = scalingDirectionStr.find(']', i);
                    if (closingIdx == std::string::npos)
                        throw ValidationException("Unmatched '[' in scaling direction");
                    std::string groupedDirections = scalingDirectionStr.substr(i + 1, closingIdx - i - 1);
                    i = closingIdx;

                    for (char direction : groupedDirections) {
                        if (direction < 'x' && direction > 'z')
                            throw ValidationException(SCALING_USAGE);
                        std::size_t directionIdx = direction - 'x';
                        if (directionUsed[directionIdx])
                            throw ValidationException("Duplicated occurrence of " + std::string{direction} + " direction");
                        directionUsed[directionIdx] = true;
                    }
                    break;
                }
                default:
                    throw ValidationException(SCALING_USAGE);
            }
        }

        ValidateMsg(std::find(directionUsed.begin(), directionUsed.end(), false) == directionUsed.end(),
                    "The behaviour of one or more scaling directions is unspecified");

        return std::make_unique<AnisotropicVolumeScaler>(std::move(factorSampler), scalingDirection, independent);
    }
}

std::unique_ptr<TriclinicBoxScaler>TriclinicBoxScalerFactory::create(const std::string &scalingType) {
    std::string scalingTypeStripped = scalingType;
    std::string independentString = "independent ";
    bool scaleTogether = !startsWith(scalingType, independentString);
    if (!scaleTogether)
        scalingTypeStripped = scalingType.substr(independentString.length());

    if (scalingTypeStripped == "delta triclinic")
        return std::make_unique<TriclinicDeltaScaler>(scaleTogether);
    else
        return std::make_unique<TriclinicAdapter>(create_volume_scaler(scalingType));
}
