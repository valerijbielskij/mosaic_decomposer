
#pragma once

#include <vector>
#include <memory>
#include "pixel.h"

class Frame
{
public:
    using PixelContainer = std::vector<std::vector<Pixel>>;
    using DimensionsType = uint16_t;

    Frame(DimensionsType width, DimensionsType height);
    Frame(const Frame& other) = default;

    Frame rotate90() const;

    void set(DimensionsType x, DimensionsType y, const Pixel& pixel);
    Pixel get(DimensionsType x, DimensionsType y) const;

    DimensionsType getWidth() const;
    DimensionsType getHeight() const;

private:
    enum class Transposition
    {
        None,
        Rotated90
    };

    Frame(const Frame& other, Transposition transposition);

    PixelContainer& get();
    const PixelContainer& get() const;

    uint16_t getActualHeight() const;
    uint16_t getActualWidth() const;

    void transpose(uint16_t& x, uint16_t& y) const;

    std::shared_ptr<PixelContainer> m_pixels;
    const Transposition m_transposition = Transposition::None;
};