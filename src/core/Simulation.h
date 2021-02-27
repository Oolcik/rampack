//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H

#include <random>
#include <iosfwd>

#include "Packing.h"
#include "utils/Logger.h"
#include "utils/Quantity.h"
#include "Interaction.h"

class Simulation {
public:
    struct ScalarSnapshot {
        std::size_t cycleCount{};
        double value{};

        friend std::ostream &operator<<(std::ostream &out, const Simulation::ScalarSnapshot &snapshot);
    };

private:
    struct Counter {
        std::size_t movesSinceEvaluation{};
        std::size_t acceptedMovesSinceEvaluation{};
        std::size_t moves{};
        std::size_t acceptedMoves{};

        void increment(bool accepted);
        void reset();
        void resetCurrent();

        [[nodiscard]] double getCurrentRate() const {
            return static_cast<double>(this->acceptedMovesSinceEvaluation) / this->movesSinceEvaluation;
        }

        [[nodiscard]] double getRate() const {
            return static_cast<double>(this->acceptedMoves) / this->moves;
        }
    };

    std::size_t thermalisationCycles{};
    std::size_t averagingCycles{};
    std::size_t averagingEvery{};
    std::size_t cycleLength{};

    double temperature{};
    double pressure{};

    double translationStep{};
    double rotationStep{};
    double scalingStep{};
    Counter translationCounter;
    Counter rotationCounter;
    Counter scalingCounter;
    bool shouldAdjustStepSize{};

    std::mt19937 mt;
    std::uniform_int_distribution<int> moveTypeDistribution;
    std::uniform_real_distribution<double> unitIntervalDistribution;
    std::uniform_int_distribution<int> particleIdxDistribution;

    std::unique_ptr<Packing> packing;
    std::vector<double> averagedDensities;
    std::vector<double> averagedEnergy;
    std::vector<double> averagedEnergyFluctuations;
    std::vector<ScalarSnapshot> densityThermalisationSnapshots;

    void performStep(Logger &logger, const Interaction &interaction);
    bool tryTranslation(const Interaction &interaction);
    bool tryRotation(const Interaction &interaction);
    bool tryMove(const Interaction &interaction);
    bool tryScaling(const Interaction &interaction);
    void evaluateCounters(Logger &logger);
    void reset();

public:
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
               double scalingStep, unsigned long seed);

    void perform(double temperature_, double pressure_, std::size_t thermalisationCycles_, std::size_t averagingCycles_,
                 std::size_t averagingEvery_, const Interaction &interaction, Logger &logger);
    [[nodiscard]] Quantity getAverageDensity() const;
    [[nodiscard]] Quantity getAverageEnergy() const;
    [[nodiscard]] Quantity getAverageEnergyFluctuations() const;
    [[nodiscard]] std::vector<ScalarSnapshot> getDensityThermalisationSnapshots() const {
        return this->densityThermalisationSnapshots;
    }
    [[nodiscard]] double getTranlationAcceptanceRate() const { return this->translationCounter.getRate(); }
    [[nodiscard]] double getRotationAcceptanceRate() const { return this->rotationCounter.getRate(); }
    [[nodiscard]] double getScalingAcceptanceRate() const { return this->scalingCounter.getRate(); }
    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
};


#endif //RAMPACK_SIMULATION_H
