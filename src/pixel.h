#pragma once

#include <cstdint>

using Color = uint8_t;

struct Pixel
{
    Color m_red{};
    Color m_green{};
    Color m_blue{};

    bool coarseCompare(const Pixel& other, uint16_t acceptable_color_deviation = 60) const;
};
