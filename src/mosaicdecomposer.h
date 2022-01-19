#pragma once

#include <utility>
#include <vector>
#include <cstdint>

#include "configparams.h"
#include "frame.h"

class FrameProviderInterface;

/**
 * @brief Core algorithm of mosaic decomposition. It can be summarized with the 3 following steps:
 *  1. Collect vertical and horizontal lines which have bigger color mismatch than the average.
 *  2. Collate adjacent lines as they are sometimes artifacts caused by misalignment on the borders of mosaics.
 *  3. Drop such lines which were detected substantially less times than the rest (false positives).
 */
class MosaicDecomposer
{
public:
    using SplitPosition = Frame::DimensionsType;

    struct SplitDimensions
    {
        SplitPosition m_x{};
        SplitPosition m_y{};
        Frame::DimensionsType m_width{};
        Frame::DimensionsType m_height{};
    };

    MosaicDecomposer(FrameProviderInterface& frame_provider, const ConfigParams& params = ConfigParams{});
    ~MosaicDecomposer() = default;

    std::vector<SplitDimensions> calculateMosaicsDimensions();

private:
    struct SampleAvgStorage
    {
        void addSample(double sample);

        uint64_t getSampleCount() const;
        double getTotalAverage() const;

    private:
        double m_average_match_rate = 0.f;
        uint64_t m_total_samples = 0;
    };

    struct SplitOccurenceData
    {
        // vertical or horizontal position of the split
        SplitPosition m_position;
        // amount of occurrences of the potential split in the context of all processed frames
        uint16_t m_match_count;
    };

    void printConfigParams() const;
    void processFrame(const Frame& frame, std::vector<SplitPosition>& potential_splits, SampleAvgStorage& comparisons) const;
    std::vector<SplitOccurenceData> collateAdjacentSplits(std::vector<SplitPosition> potential_splits) const;
    std::vector<SplitPosition> dropFalsePositiveSplits(const std::vector<SplitOccurenceData>& potential_splits) const;

    std::vector<SplitDimensions> translate(
        const std::vector<SplitPosition>& horizontal_positions, const std::vector<SplitPosition>& vertical_positions) const;

    FrameProviderInterface& m_frame_provider;
    const ConfigParams m_params;
};