#pragma once

#include <cstdint>

struct Pixel
{
    using Color = uint8_t;

    Color m_red{};
    Color m_green{};
    Color m_blue{};

    bool coarseCompare(const Pixel& other, uint16_t acceptable_color_deviation = 80) const;
};
