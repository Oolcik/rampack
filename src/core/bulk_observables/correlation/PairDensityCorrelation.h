//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRDENSITYCORRELATION_H
#define RAMPACK_PAIRDENSITYCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "Histogram.h"


class PairDensityCorrelation : public BulkObservable, public PairConsumer {
private:
    std::unique_ptr<PairEnumerator> pairEnumerator;
    Histogram histogram;

public:
    explicit PairDensityCorrelation(std::unique_ptr<PairEnumerator> pairEnumerator, double maxR, std::size_t numBins)
            : pairEnumerator{std::move(pairEnumerator)}, histogram(0, maxR, numBins)
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override { this->histogram.clear(); }
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair, double distance,
                     double jacobian) override;
};


#endif //RAMPACK_PAIRDENSITYCORRELATION_H
