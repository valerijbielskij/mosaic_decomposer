#pragma once

#include <utility>
#include <vector>
#include <cstdint>

#include "configparams.h"

class FrameProviderInterface;
class Frame;
    
class MosaicDecomposer
{
public:

    using SplitPosition = uint16_t; // TODO static assert with frame size?

    struct SplitInfo
    {
        SplitPosition m_x{};
        SplitPosition m_y{};
        uint16_t m_width{};
        uint16_t m_height{};
    };

    MosaicDecomposer(FrameProviderInterface& frame_provider, const ConfigParams& params = ConfigParams{});
    ~MosaicDecomposer() = default;

    std::vector<SplitInfo> calculateMosaicsDimensions();

private:
    struct LineComparisonData
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
        // match rate of successful pixel comparisons with regards to the previous position // TODO update
        uint16_t m_match_count;
    };

    void printConfigParams() const;
    void processFrame(const Frame& frame, std::vector<SplitPosition>& potential_splits, LineComparisonData& global_comparisons) const;
    std::vector<SplitOccurenceData> collateAdjacentSplits(std::vector<SplitPosition> potential_splits) const;
    std::vector<SplitPosition> dropFalsePositiveSplits(const std::vector<SplitOccurenceData>& potential_splits) const;

    std::vector<SplitInfo> translate(
        const std::vector<SplitPosition>& horizontal_positions, const std::vector<SplitPosition>& vertical_positions) const;

    FrameProviderInterface& m_frame_provider;
    ConfigParams m_params;
};