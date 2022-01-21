#include  <numeric>

#include <spdlog/spdlog.h>

#include "mosaicdecomposer.h"
#include "frameproviderinterface.h"

constexpr uint8_t MINIMUM_AMOUNT_OF_SAMPLES = 10;

MosaicDecomposer::MosaicDecomposer(FrameProviderInterface& frame_provider, const ConfigParams& params) : 
    m_frame_provider(frame_provider), 
    m_params(params)
{
}

void MosaicDecomposer::printConfigParams() const
{
    spdlog::info("starting printing configuration parameters");

    spdlog::info("{:<20} = {}", "frames_to_analyze", m_params.m_frames_to_analyze);
    spdlog::info("{:<20} = {}", "skip_front_lines", m_params.m_skip_front_lines);
    spdlog::info("{:<20} = {}", "skip_back_lines", m_params.m_skip_back_lines);
    spdlog::info("{:<20} = {}", "pixel_match_ratio", m_params.m_minimum_pixel_match_ratio);
    spdlog::info("{:<20} = {}", "color_match_diff", m_params.m_minimum_color_match_diff);
    spdlog::info("{:<20} = {}", "line_match_ratio", m_params.m_minimum_line_match_ratio);

    spdlog::info("finished printing configuration parameters");
}

std::vector<MosaicDecomposer::SplitDimensions> MosaicDecomposer::calculateMosaicsDimensions()
{
    if (!m_frame_provider.isReady())
    {
        spdlog::error("frame provider is not ready");
        return {};
    }

    printConfigParams();

    std::vector<SplitPosition> potential_horizontal_splits;
    std::vector<SplitPosition> potential_vertical_splits;

    SampleAvgStorage horizontal_comparisons;
    SampleAvgStorage vertical_comparisons;

    Frame::DimensionsType width = 0;
    Frame::DimensionsType height = 0;

    uint32_t processed_frames = 0;

    spdlog::info("starting frame analysis");

    while (const auto& frame = m_frame_provider.getNext())
    {
        if (width == 0)
        {
            width = frame->getWidth();
        }
        else if (width != frame->getWidth())
        {
            spdlog::error("frames width is not consistent");
            return {};
        }

        if (height == 0)
        {
            height = frame->getHeight();
        }
        else if (height != frame->getHeight())
        {
            spdlog::error("frames height is not consistent");
            return {};
        }

        processFrame(*frame, potential_horizontal_splits, horizontal_comparisons);

        const auto& rotated = frame->rotate90();
        processFrame(rotated, potential_vertical_splits, vertical_comparisons);

        if (++processed_frames == m_params.m_frames_to_analyze)
        {
            spdlog::info("reached requested amount of frames to analyze, stopping");
            break;
        }
    }

    spdlog::info("finished analyzing frames, total amount: {}", processed_frames);

    spdlog::info("starting processing potential horizontal splits");
    auto collated_horizontal_splits = collateAdjacentSplits(potential_horizontal_splits);
    auto filtered_horizontal_splits = dropFalsePositiveSplits(collated_horizontal_splits);

    spdlog::info("starting processing potential vertical splits");
    auto collated_vertical_splits = collateAdjacentSplits(potential_vertical_splits);
    auto filtered_vertical_splits = dropFalsePositiveSplits(collated_vertical_splits);

    // front and back positions must be manually added in order to calculate mosaic dimensions
    filtered_horizontal_splits.insert(filtered_horizontal_splits.begin(), 0);
    filtered_horizontal_splits.push_back(height);

    filtered_vertical_splits.insert(filtered_vertical_splits.begin(), 0);
    filtered_vertical_splits.push_back(width);

    return translate(filtered_horizontal_splits, filtered_vertical_splits);
}

void MosaicDecomposer::processFrame(const Frame& frame, std::vector<SplitPosition>& potential_splits, 
    SampleAvgStorage& comparisons) const
{
    potential_splits.resize(frame.getHeight());

    // micro optimization since these methods might need to transpose the size
    const auto frame_width = frame.getWidth();
    const auto frame_height = frame.getHeight();

    const auto length = frame_height - (m_params.m_skip_back_lines ? m_params.m_skip_back_lines : 1);
    for (SplitPosition i = m_params.m_skip_front_lines; i < length; i++)
    {
        Frame::DimensionsType matched_pixels = 0;

        for (SplitPosition j = 0; j < frame_width; j++)
        {
            auto current_pixel = frame.get(j, i);
            auto next_pixel = frame.get(j, static_cast<SplitPosition>(i + 1));

            if (current_pixel.coarseCompare(next_pixel, m_params.m_minimum_color_match_diff))
            {
                matched_pixels++;
            }
        }

        const auto current_line_match_rate = static_cast<double>(matched_pixels) / frame_width * 100.;
        const auto is_rate_below_acceptable = current_line_match_rate < 
                (comparisons.getTotalAverage() / m_params.m_minimum_pixel_match_ratio);

        if (comparisons.getSampleCount() > MINIMUM_AMOUNT_OF_SAMPLES && is_rate_below_acceptable)
        {
            potential_splits[i]++;
        }
        
        comparisons.addSample(current_line_match_rate);
    }
}

std::vector<MosaicDecomposer::SplitOccurenceData> MosaicDecomposer::collateAdjacentSplits(
    std::vector<SplitOccurenceType> potential_splits) const
{
    std::vector<SplitOccurenceData> collated_splits;

    auto update = [](SplitOccurenceType& current_value, SplitOccurenceType& next_value, 
            SplitPosition curr_index, SplitPosition next_index)
    {
        if (next_value > 0)
        {
            if (current_value < next_value)
            {
                next_value = static_cast<SplitOccurenceType>(next_value + current_value);
                current_value = 0;
                spdlog::info("collated value of position {} into position {}, new match count is {}", 
                        curr_index, next_index, next_value);
            }
            else
            {
                current_value = static_cast<SplitOccurenceType>(current_value + next_value);
                next_value = 0;
                spdlog::info("collated value of position {} into position {}, new match count is {}", 
                        next_index, curr_index, current_value);
            }
        }    
    };

    for (SplitPosition i = 0; i < static_cast<SplitPosition>(potential_splits.size()) - 1; i++)
    {
        auto& current_match_count = potential_splits[i];
        const auto next_index = static_cast<SplitPosition>(i + 1);
        auto& next_match_count = potential_splits[next_index];
        
        if (current_match_count == 0)
        {
            continue;
        }

        if (next_match_count > 0)
        {
            update(current_match_count, next_match_count, i, next_index);
        }

        if (current_match_count && i < static_cast<SplitPosition>(potential_splits.size()) - 2)
        {
            const auto next_next_index = static_cast<SplitPosition>(i + 2);
            auto& next_next_match_count = potential_splits[next_next_index];

            update(current_match_count, next_next_match_count, i, next_next_index);
        }

        if (current_match_count > 0)
        {
            collated_splits.push_back(SplitOccurenceData{i, current_match_count});
        }
    }

    return collated_splits;
}

std::vector<MosaicDecomposer::SplitPosition> MosaicDecomposer::dropFalsePositiveSplits(
    const std::vector<SplitOccurenceData>& potential_splits) const
{
    std::vector<SplitPosition> filtered_splits;

    const auto sum = std::accumulate(potential_splits.begin(), potential_splits.end(), 0.,
        [](double accumulated_value, const SplitOccurenceData& data) -> double
    {
        return accumulated_value + data.m_match_count;
    });

    const auto total_average = sum / static_cast<double>(potential_splits.size());

    for (const auto& data : potential_splits)
    {
        const auto minimum_count = total_average / m_params.m_minimum_line_match_ratio;
        if ( data.m_match_count >= minimum_count)
        {
            filtered_splits.push_back(data.m_position);
        }
        else
        {
            spdlog::info("position {} is considered a false positive, occured only {} times, vs average of {}", 
                        data.m_position, data.m_match_count, total_average);
        }
    }

    return filtered_splits;
}

std::vector<MosaicDecomposer::SplitDimensions> MosaicDecomposer::translate(
    const std::vector<SplitPosition>& horizontal_positions, const std::vector<SplitPosition>& vertical_positions) const
{
    std::vector<SplitDimensions> splits;

    for (SplitPosition j = 0; j < horizontal_positions.size() - 1; j++)
    {
            for (SplitPosition i = 0; i < vertical_positions.size() - 1; i++)
            {
            const auto current_v = vertical_positions[i];
            const auto next_v = vertical_positions[static_cast<SplitPosition>(i + 1)];

            const auto current_h = horizontal_positions[j];
            const auto next_h = horizontal_positions[static_cast<SplitPosition>(j + 1)];

            if (current_h >= next_h)
            {
                spdlog::error("horizontal positions were supplied not in an ascending order");
                return {};
            }

            if (current_v >= next_v)
            {
                spdlog::error("vertical positions were supplied not in an ascending order");
                return {};
            }

            const auto width = static_cast<Frame::DimensionsType>(next_v - current_v);
            const auto height = static_cast<Frame::DimensionsType>(next_h - current_h);

            splits.push_back(SplitDimensions{current_v, current_h, width, height});
        }
    }

    return splits;
}


void MosaicDecomposer::SampleAvgStorage::addSample(double sample)
{
    const auto previous_sum = m_average_match_rate * static_cast<double>(m_total_samples);
    m_total_samples++;
    m_average_match_rate = (previous_sum + sample) / static_cast<double>(m_total_samples);
}

double MosaicDecomposer::SampleAvgStorage::getTotalAverage() const 
{
    return m_average_match_rate;
}

uint64_t MosaicDecomposer::SampleAvgStorage::getSampleCount() const
{
    return m_total_samples;
}
