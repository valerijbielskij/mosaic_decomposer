#pragma once

#include <cstdint>

using SplitPosition = uint16_t; // TODO static assert with frame size?

struct ConfigParams
{
    //! @brief Defines amount of frame to analyze, 0 means analyze all available frames.
    uint32_t m_frames_to_analyze = 0;

    //! @brief Defines amount of lines from the front of the frame to be skipped during processing
    SplitPosition m_skip_front_lines = 5;

    //! @brief Defines amount of lines from the back of the frame to be skipped during processing
    SplitPosition m_skip_back_lines = 5;

    //! @brief Defines ratio used to decide whether a line shall be considered as potential split line.
    //! For ex. when set to 1.5 if current global average match rate is 80%, then subsequent matches with 
    //! rates lesser than 60% will be considered as splits, contrarirly matches with rate above 60% won't be.
    //! @note Match ratio is calculated by counting all pixels which (relatively) 
    //! match with the adjacent pixel on the next line.
    double m_minimum_pixel_match_ratio = 1.4;

    //! @brief Defines minimum difference of color units of pixel to be considered as relatively matched.
    //! For ex. rgb(255, 255, 255) and rgb(250, 250, 255) has a difference of 10 units of color.
    uint16_t m_minimum_color_match_diff = 100;

    //! @brief Defines ratio used to decide whether a line shall be considered as false positive or not,
    //! it helps to filter out lines which were detected by an accident.
    //! For ex. when set to 1.5 if current average match rate is 80%, then subsequent matches with 
    //! rates lesser than 60% will be considered as false positives and dropped, 
    //! contrarirly matches with rate above 60% will be kept.
    double m_minimum_line_match_ratio = 1.5;
};