//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_HISTOGRAM_H
#define RAMPACK_HISTOGRAM_H

#include <ostream>
#include <vector>


class Histogram {
private:
    struct BinData {
        double value{};
        std::size_t numPoints{};

        void addPoint(double newValue);

        BinData &operator+=(const BinData &other);
    };

    struct HistogramData {
        std::vector<BinData> bins;

        explicit HistogramData(std::size_t numBins) : bins(numBins) { }

        HistogramData &operator+=(const HistogramData &otherData);
        [[nodiscard]] std::size_t size() const { return bins.size(); }
        void clear();
    };

    double min{};
    double max{};
    double step{};
    std::size_t numSnapshots{};
    HistogramData histogram;
    HistogramData currentHistogram;
    std::vector<double> binValues{};

public:
    enum class ReductionMethod {
        SUM,
        AVERAGE
    };

    explicit Histogram(double min, double max, std::size_t numBins);

    void add(double value, double pos);
    void nextSnapshot();
    [[nodiscard]] std::vector<std::pair<double, double>> dumpValues(ReductionMethod reductionMethod) const;
    void clear();
    [[nodiscard]] std::size_t size() const { return this->histogram.size(); }
    [[nodiscard]] double getBinSize() const { return this->step; }
    [[nodiscard]] double getMin() const { return this->min; }
    [[nodiscard]] double getMax() const { return this->max; }
};


#endif //RAMPACK_HISTOGRAM_H
