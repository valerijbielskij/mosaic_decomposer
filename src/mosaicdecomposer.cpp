#include "mosaicdecomposer.h"
#include "frameproviderinterface.h"
#include  <numeric>
#include <iostream>
#include "frame.h"

MosaicDecomposer::MosaicDecomposer(FrameProviderInterface& frame_provider, const ConfigParams& params) : m_frame_provider(frame_provider), m_params(params)
{
}

std::vector<MosaicDecomposer::SplitInfo> MosaicDecomposer::calculateMosaicsDimensions()
{
    if (!m_frame_provider.isReady())
    {
        // TODO log
        std::cerr << "1\n" << std::flush;
        return {};
    }

    std::vector<SplitPosition> potential_horizontal_splits;
    std::vector<SplitPosition> potential_vertical_splits;

    LineComparisonData horizontal_comparisons;
    LineComparisonData vertical_comparisons;

    uint16_t width = 0;
    uint16_t height = 0;

    uint32_t processed_frames = 0; // TODO limit amount?

    while (const auto& frame = m_frame_provider.getNext())
    {
        if (width == 0)
        {
            width = frame->getWidth();
        }
        else if (width != frame->getWidth())
        {
            // log error
            std::cerr << "2\n" << std::flush;
            return {};
        }

        if (height == 0)
        {
            height = frame->getHeight();
        }
        else if (height != frame->getHeight())
        {
            // log error
            std::cerr << "3\n" << std::flush;
            return {};
        }

        processFrame(*frame, potential_horizontal_splits, horizontal_comparisons);

        const auto& rotated = frame->rotate90();
        processFrame(rotated, potential_vertical_splits, vertical_comparisons);

        processed_frames++;
    }

    auto collated_horizontal_splits = collateAdjacentSplits(potential_horizontal_splits);
    auto collated_vertical_splits = collateAdjacentSplits(potential_vertical_splits);

    auto filtered_horizontal_splits = dropFalsePositiveSplits(collated_horizontal_splits);
    auto filtered_vertical_splits = dropFalsePositiveSplits(collated_vertical_splits);

    // TODO add comment
    filtered_horizontal_splits.insert(filtered_horizontal_splits.begin(), 0);
    filtered_horizontal_splits.push_back(height);

    filtered_vertical_splits.insert(filtered_vertical_splits.begin(), 0);
    filtered_vertical_splits.push_back(width);

    return translate(filtered_horizontal_splits, filtered_vertical_splits);
}

void MosaicDecomposer::processFrame(const Frame& frame, std::vector<SplitPosition>& potential_splits, LineComparisonData& global_comparisons) const
{
    potential_splits.resize(frame.getHeight());

    //std::cout << "w= " << frame.getWidth() << "h= " << frame.getHeight();

    std::vector<double> match_rates_of_processed_positions;

    // micro optimization since these methods might need to transpose size
    const auto frame_width = frame.getWidth();
    const auto frame_height = frame.getHeight();

    SplitPosition skip_front = 5;
    SplitPosition skip_back = 5;

    for (SplitPosition i = skip_front; i < frame_height - (skip_back ? skip_back : 1); i++)
    {
        Frame::DimensionsType matched_pixels = 0;

        for (SplitPosition j = 0; j < frame_width; j++)
        {
            auto current_pixel = frame.get(j, i);
            auto next_pixel = frame.get(j, i + 1);

            if (current_pixel.coarseCompare(next_pixel, 100))
            {
                matched_pixels++;
            }
        }

        const auto average = global_comparisons.getTotalAverage();

        const auto current_line_match_rate = static_cast<double>(matched_pixels) / frame_width * 100.;

        if (global_comparisons.getSampleCount() > 10 && current_line_match_rate < (average / 1.4)) // TODO magic const
        {
            // TODO assert size?
            //std::cout << "hh" << i << "\n" << std::flush;
            potential_splits[i]++;
        }
        
        global_comparisons.addSample(current_line_match_rate);
        
        
        /*
        //std::cout << "pf: " << i << ", matched_pixels= " << matched_pixels << ", avg= " << average << ", curr= " << current_line_match_rate << std::endl;

        // TODO remove min gap concept?
        // TODO use global avg of all frames?
        // TODO consider min gap
        if (match_rates_of_processed_positions.size() < 20)
        {
            match_rates_of_processed_positions.push_back(current_line_match_rate);
            continue;
        }

        //const auto average = std::accumulate(match_rates_of_processed_positions.begin(), match_rates_of_processed_positions.end(), 0.f) 
        //    / static_cast<float>(match_rates_of_processed_positions.size());

        if (current_line_match_rate < (average / 1.4)) // TODO magic const
        {
            // TODO assert size?
            std::cout << "hh" << i << "\n" << std::flush;
            potential_splits[i]++;
            match_rates_of_processed_positions.clear(); // TODO clarify
        }
        else
        {
            match_rates_of_processed_positions.push_back(current_line_match_rate);
        }
        */
    }
}

std::vector<MosaicDecomposer::SplitOccurenceData> MosaicDecomposer::collateAdjacentSplits(std::vector<SplitPosition> potential_splits) const
{
    std::cout << "potential_splits: " << potential_splits.size() << "\n\n" << std::flush;

    std::vector<SplitOccurenceData> collated_splits;

    auto update = [](SplitPosition& current_value, SplitPosition& next_value)
    {
        if (next_value > 0)
        {
            if (current_value < next_value)
            {
                next_value += current_value;
                current_value = 0;
                // TODO log
                std::cerr << "4\n" << std::flush;
            }
            else
            {
                current_value += next_value;
                next_value = 0;
                std::cerr << "5\n" << std::flush;
            }
        }    
    };

    for (SplitPosition i = 0; i < static_cast<SplitPosition>(potential_splits.size()) - 1; i++)
    {
        auto& current_match_count = potential_splits[i];
        auto& next_match_count = potential_splits[i + 1];
        
        if (current_match_count == 0)
        {
            continue;
        }

        if (next_match_count > 0)
        {
            update(current_match_count, next_match_count);
        }

        if (current_match_count && i < static_cast<SplitPosition>(potential_splits.size()) - 2)
        {
            auto& next_next_match_count = potential_splits[i + 2];

            update(current_match_count, next_next_match_count);
        }

        if (current_match_count > 0)
        {
            collated_splits.push_back(SplitOccurenceData{i, current_match_count});
        }
    }

    return collated_splits;
}

std::vector<MosaicDecomposer::SplitPosition> MosaicDecomposer::dropFalsePositiveSplits(const std::vector<SplitOccurenceData>& potential_splits) const
{
    std::cout << "potential_splits: " << potential_splits.size() << "\n\n" << std::flush;

    std::vector<SplitPosition> filtered_splits;

    const auto sum = std::accumulate(potential_splits.begin(), potential_splits.end(), 0.f, [](float accumulated_value, const SplitOccurenceData& data) -> float
    {
        return accumulated_value + data.m_match_count;
    });

    const auto total_average = sum / static_cast<float>(potential_splits.size());

    for (const auto& data : potential_splits)
    {
        if ( data.m_match_count > (total_average / 1.5f)) // TODO magic const
        {
            filtered_splits.push_back(data.m_position);
        }
        else
        {
            // TODO log
            std::cerr << "6666\n" << std::flush;
        }
    }

    return filtered_splits;
}

std::vector<MosaicDecomposer::SplitInfo> MosaicDecomposer::translate(
        const std::vector<SplitPosition>& horizontal_positions, const std::vector<SplitPosition>& vertical_positions) const
{
    std::cout << "h1: ";
    for(const auto& h : horizontal_positions)
    {
        std::cout << h << " ";
    }
    std::cout << std::endl;

     std::cout << "v1: ";
    for(const auto& v : vertical_positions)
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    std::cout << "horizontal_positions: " << horizontal_positions.size() << "\n" << std::flush;
    std::cout << "vertical_positions: " << vertical_positions.size() << "\n" << std::flush;

    std::vector<SplitInfo> splits;

    for (SplitPosition j = 0; j < horizontal_positions.size() - 1; j++)
    {
            for (SplitPosition i = 0; i < vertical_positions.size() - 1; i++)
            {
            const auto current_v = vertical_positions[i];
            const auto next_v = vertical_positions[i + 1];

            const auto current_h = horizontal_positions[j];
            const auto next_h = horizontal_positions[j + 1];

            if (current_h >= next_h)
            {
                std::cerr << "7\n" << std::flush;
                return {}; // log
            }

            if (current_v >= next_v)
            {
                std::cerr << "8\n" << std::flush;
                return {}; // log
            }

            const auto width = static_cast<uint16_t>(next_v - current_v);
            const auto height = static_cast<uint16_t>(next_h - current_h);

            splits.push_back(SplitInfo{current_v, current_h, width, height});
        }
    }

    return splits;
}


void MosaicDecomposer::LineComparisonData::addSample(double sample)
{
    const auto previous_sum = m_average_match_rate * static_cast<double>(m_total_samples);
    m_total_samples++;
    m_average_match_rate = (previous_sum + sample) / static_cast<double>(m_total_samples);
}

double MosaicDecomposer::LineComparisonData::getTotalAverage() const 
{
    return m_average_match_rate;
}

uint64_t MosaicDecomposer::LineComparisonData::getSampleCount() const
{
    return m_total_samples;
}
