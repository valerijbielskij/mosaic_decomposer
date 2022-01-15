#include "pixel.h"
#include <cstdlib>
 
bool Pixel::coarseCompare(const Pixel& other, uint16_t acceptable_color_deviation) const 
{
    auto r_diff = std::abs(m_red   - other.m_red);
    auto g_diff = std::abs(m_green - other.m_green);
    auto b_diff = std::abs(m_blue  - other.m_blue);

    auto sum = static_cast<uint16_t>(r_diff + g_diff + b_diff);

    return sum <= acceptable_color_deviation;
}